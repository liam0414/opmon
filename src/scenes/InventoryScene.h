#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include "game/Inventory.h"
#include <memory>
#include <vector>

class InventoryScene : public Scene {
private:
    // UI Elements
    std::unique_ptr<Button> backButton;
    sf::RectangleShape backgroundPanel;
    sf::Text titleText;
    sf::Text berryText;
    sf::Text itemInfoText;
    
    // Item display
    std::vector<sf::RectangleShape> itemSlots;
    std::vector<sf::Text> itemNames;
    std::vector<sf::Text> itemQuantities;
    
    // Selection
    int selectedSlot;
    ItemStack* selectedItem;
    
    // Pagination
    int currentPage;
    int itemsPerPage;
    int totalPages;
    
    void setupUI();
    void updateItemDisplay();
    void updateItemInfo();
    void handleItemSelection(int slotIndex);
    void useSelectedItem();
    
public:
    InventoryScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};