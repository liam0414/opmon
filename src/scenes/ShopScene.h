#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include "game/Item.h"
#include <memory>
#include <vector>

struct ShopItem {
    std::unique_ptr<Item> item;
    int price;
    int stock; // -1 for unlimited
    bool available;
    
    ShopItem(std::unique_ptr<Item> i, int p, int s = -1) 
        : item(std::move(i)), price(p), stock(s), available(true) {}
};

enum class ShopMode {
    Buy,
    Sell
};

class ShopScene : public Scene {
private:
    // Shop data
    std::vector<std::unique_ptr<ShopItem>> shopInventory;
    std::string shopName;
    std::string shopkeeper;
    
    // UI Elements
    std::unique_ptr<Button> buyModeButton;
    std::unique_ptr<Button> sellModeButton;
    std::unique_ptr<Button> buyButton;
    std::unique_ptr<Button> sellButton;
    std::unique_ptr<Button> exitButton;
    
    std::vector<sf::RectangleShape> itemSlots;
    std::vector<sf::Text> itemNames;
    std::vector<sf::Text> itemPrices;
    std::vector<sf::Text> itemStock;
    
    sf::Text titleText;
    sf::Text shopkeeperText;
    sf::Text playerBerryText;
    sf::Text itemInfoText;
    sf::RectangleShape backgroundPanel;
    sf::RectangleShape infoPanel;
    
    // State
    ShopMode currentMode;
    int selectedItemIndex;
    int quantityToTrade;
    
    // Pagination
    int currentPage;
    int itemsPerPage;
    
    void setupUI();
    void updateItemDisplay();
    void updatePlayerInfo();
    void selectItem(int index);
    void buySelectedItem();
    void sellSelectedItem();
    void switchMode(ShopMode mode);
    void showItemInfo();
    
    // Shop inventory management
    void addShopItem(std::unique_ptr<Item> item, int price, int stock = -1);
    void createDefaultShop();

public:
    ShopScene(const std::string& name, const std::string& keeper);
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    
    // Shop setup
    void setShopInventory(std::vector<std::unique_ptr<ShopItem>> inventory);
    
    // Static factory methods
    static std::unique_ptr<ShopScene> createGeneralStore();
    static std::unique_ptr<ShopScene> createWeaponShop();
    static std::unique_ptr<ShopScene> createFoodShop();
};