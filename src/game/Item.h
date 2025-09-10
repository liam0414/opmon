#pragma once
#include <string>
#include <nlohmann/json.hpp>

enum class ItemType {
    Consumable,  // Food, potions, etc.
    Weapon,      // Swords, guns, etc.
    Armor,       // Defensive equipment
    Accessory,   // Rings, necklaces, etc.
    KeyItem,     // Story items, keys, etc.
    Treasure,    // Valuable items for selling
    Material     // Crafting materials
};

enum class ItemRarity {
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary
};

class Character;

class Item {
protected:
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    ItemRarity rarity;
    
    int value; // Sell price in berries
    int stackSize; // Max stack size (1 for unique items)
    std::string iconTexture;
    
    // Usage effects
    bool consumable;
    std::function<void(Character*)> useEffect;

public:
    Item(const std::string& itemId, const std::string& itemName, ItemType itemType, ItemRarity itemRarity = ItemRarity::Common);
    virtual ~Item() = default;
    
    // Basic info
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    ItemType getType() const { return type; }
    ItemRarity getRarity() const { return rarity; }
    
    void setDescription(const std::string& desc) { description = desc; }
    void setIconTexture(const std::string& texture) { iconTexture = texture; }
    
    // Value and stacking
    int getValue() const { return value; }
    void setValue(int val) { value = val; }
    int getStackSize() const { return stackSize; }
    void setStackSize(int size) { stackSize = size; }
    
    // Usage
    bool isConsumable() const { return consumable; }
    void setConsumable(bool cons) { consumable = cons; }
    virtual bool canUse(Character* user) const;
    virtual void use(Character* user);
    void setUseEffect(std::function<void(Character*)> effect) { useEffect = effect; }
    
    // Visual
    const std::string& getIconTexture() const { return iconTexture; }
    
    // Serialization
    virtual nlohmann::json toJson() const;
    virtual void fromJson(const nlohmann::json& data);
    
    // Utility
    std::string getRarityColor() const;
    static std::unique_ptr<Item> createFromJson(const nlohmann::json& data);
};

struct ItemStack {
    std::unique_ptr<Item> item;
    int quantity;
    
    ItemStack(std::unique_ptr<Item> i, int q = 1) : item(std::move(i)), quantity(q) {}
    
    bool canAddMore() const {
        return quantity < item->getStackSize();
    }
    
    int addItems(int amount) {
        int maxAdd = item->getStackSize() - quantity;
        int actualAdd = std::min(amount, maxAdd);
        quantity += actualAdd;
        return amount - actualAdd; // Return remaining items that couldn't be added
    }
    
    bool removeItems(int amount) {
        if (amount <= quantity) {
            quantity -= amount;
            return true;
        }
        return false;
    }
};

class Inventory {
private:
    std::vector<std::unique_ptr<ItemStack>> items;
    int maxSlots;
    long berry; // Currency

public:
    Inventory(int slots = 50);
    
    // Currency
    long getBerry() const { return berry; }
    void addBerry(long amount) { berry += amount; }
    bool spendBerry(long amount);
    
    // Item management
    bool addItem(std::unique_ptr<Item> item, int quantity = 1);
    bool removeItem(const std::string& itemId, int quantity = 1);
    bool hasItem(const std::string& itemId, int quantity = 1) const;
    int getItemQuantity(const std::string& itemId) const;
    
    // Access
    const std::vector<std::unique_ptr<ItemStack>>& getItems() const { return items; }
    ItemStack* getItemStack(const std::string& itemId);
    std::vector<ItemStack*> getItemsByType(ItemType type);
    
    // Usage
    bool useItem(const std::string& itemId, Character* user);
    
    // Capacity
    int getUsedSlots() const { return items.size(); }
    int getMaxSlots() const { return maxSlots; }
    int getFreeSlots() const { return maxSlots - items.size(); }
    bool isFull() const { return getUsedSlots() >= maxSlots; }
    
    // Sorting and organization
    void sortByName();
    void sortByType();
    void sortByRarity();
    void sortByValue();
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
    
    // Utility
    long getTotalValue() const;
    void clear();
};