#include "InventoryScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "game/GameStateManager.h"

InventoryScene::InventoryScene() 
    : selectedSlot(-1), selectedItem(nullptr), currentPage(0), itemsPerPage(20), totalPages(1) {
    setupUI();
}

void InventoryScene::onEnter() {
    LOG_INFO("Opened inventory");
    updateItemDisplay();
}

void InventoryScene::onExit() {
    LOG_INFO("Closed inventory");
}

void InventoryScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Background panel
    backgroundPanel.setSize(sf::Vector2f(900, 650));
    backgroundPanel.setPosition(62, 59);
    backgroundPanel.setFillColor(sf::Color(40, 40, 60, 240));
    backgroundPanel.setOutlineColor(sf::Color::White);
    backgroundPanel.setOutlineThickness(2);
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("INVENTORY");
    titleText.setCharacterSize(32);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(80, 80);
    
    // Berry display
    berryText.setFont(fontManager.getDefaultFont());
    berryText.setCharacterSize(24);
    berryText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    berryText.setPosition(80, 120);
    
    // Item info panel
    itemInfoText.setFont(fontManager.getDefaultFont());
    itemInfoText.setCharacterSize(18);
    itemInfoText.setFillColor(sf::Color::White);
    itemInfoText.setPosition(600, 180);
    
    // Create item slots (4x5 grid)
    float slotSize = 60.0f;
    float slotSpacing = 70.0f;
    float startX = 100.0f;
    float startY = 180.0f;
    
    itemSlots.clear();
    itemNames.clear();
    itemQuantities.clear();
    
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 5; ++col) {
            // Item slot background
            sf::RectangleShape slot;
            slot.setSize(sf::Vector2f(slotSize, slotSize));
            slot.setPosition(startX + col * slotSpacing, startY + row * slotSpacing);
            slot.setFillColor(sf::Color(60, 60, 80, 200));
            slot.setOutlineColor(sf::Color::Gray);
            slot.setOutlineThickness(1);
            itemSlots.push_back(slot);
            
            // Item name text
            sf::Text nameText;
            nameText.setFont(fontManager.getDefaultFont());
            nameText.setCharacterSize(12);
            nameText.setFillColor(sf::Color::White);
            nameText.setPosition(startX + col * slotSpacing + 2, startY + row * slotSpacing + slotSize + 2);
            itemNames.push_back(nameText);
            
            // Item quantity text
            sf::Text quantityText;
            quantityText.setFont(fontManager.getDefaultFont());
            quantityText.setCharacterSize(14);
            quantityText.setFillColor(sf::Color::Yellow);
            quantityText.setPosition(startX + col * slotSpacing + slotSize - 15, startY + row * slotSpacing + slotSize - 15);
            itemQuantities.push_back(quantityText);
        }
    }
    
    // Back button
    backButton = std::make_unique<Button>("Close", fontManager.getDefaultFont());
    backButton->setBounds(sf::FloatRect(800, 650, 120, 40));
    backButton->setColors(sf::Color(100, 50, 50, 200), sf::Color(120, 70, 70, 220),
                         sf::Color(80, 30, 30, 240), sf::Color::Gray);
    backButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
}

void InventoryScene::updateItemDisplay() {
    auto& inventory = GameStateManager::getInstance().getInventory();
    const auto& items = inventory.getItems();
    
    // Update berry display
    berryText.setString("Berry: " + std::to_string(inventory.getBerry()) + " ฿");
    
    // Calculate pagination
    totalPages = (items.size() + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;
    
    // Clear current display
    for (size_t i = 0; i < itemSlots.size(); ++i) {
        itemSlots[i].setFillColor(sf::Color(60, 60, 80, 200));
        itemSlots[i].setOutlineColor(sf::Color::Gray);
        itemNames[i].setString("");
        itemQuantities[i].setString("");
    }
    
    // Display items for current page
    size_t startIndex = currentPage * itemsPerPage;
    size_t endIndex = std::min(startIndex + itemsPerPage, items.size());
    
    for (size_t i = startIndex; i < endIndex; ++i) {
        size_t slotIndex = i - startIndex;
        if (slotIndex >= itemSlots.size()) break;
        
        const auto& itemStack = items[i];
        if (itemStack && itemStack->item) {
            // Set slot colors based on rarity
            sf::Color slotColor;
            switch (itemStack->item->getRarity()) {
                case ItemRarity::Common:    slotColor = sf::Color(100, 100, 100, 200); break;
                case ItemRarity::Uncommon:  slotColor = sf::Color(0, 150, 0, 200); break;
                case ItemRarity::Rare:      slotColor = sf::Color(0, 100, 200, 200); break;
                case ItemRarity::Epic:      slotColor = sf::Color(150, 0, 150, 200); break;
                case ItemRarity::Legendary: slotColor = sf::Color(255, 165, 0, 200); break;
            }
            itemSlots[slotIndex].setFillColor(slotColor);
            
            // Truncate long names
            std::string displayName = itemStack->item->getName();
            if (displayName.length() > 8) {
                displayName = displayName.substr(0, 8) + "...";
            }
            itemNames[slotIndex].setString(displayName);
            
            // Show quantity if > 1
            if (itemStack->quantity > 1) {
                itemQuantities[slotIndex].setString(std::to_string(itemStack->quantity));
            }
        }
    }
    
    updateItemInfo();
}

void InventoryScene::updateItemInfo() {
    if (selectedItem && selectedItem->item) {
        std::string info = "=== " + selectedItem->item->getName() + " ===\n\n";
        info += selectedItem->item->getDescription() + "\n\n";
        info += "Type: ";
        
        switch (selectedItem->item->getType()) {
            case ItemType::Consumable: info += "Consumable"; break;
            case ItemType::Weapon: info += "Weapon"; break;
            case ItemType::Armor: info += "Armor"; break;
            case ItemType::Accessory: info += "Accessory"; break;
            case ItemType::KeyItem: info += "Key Item"; break;
            case ItemType::Treasure: info += "Treasure"; break;
            case ItemType::Material: info += "Material"; break;
        }
        
        info += "\nRarity: ";
        switch (selectedItem->item->getRarity()) {
            case ItemRarity::Common: info += "Common"; break;
            case ItemRarity::Uncommon: info += "Uncommon"; break;
            case ItemRarity::Rare: info += "Rare"; break;
            case ItemRarity::Epic: info += "Epic"; break;
            case ItemRarity::Legendary: info += "Legendary"; break;
        }
        
        info += "\nValue: " + std::to_string(selectedItem->item->getValue()) + " Berry";
        info += "\nQuantity: " + std::to_string(selectedItem->quantity);
        
        if (selectedItem->item->isConsumable()) {
            info += "\n\nPress ENTER to use";
        }
        
        itemInfoText.setString(info);
    } else {
        itemInfoText.setString("Select an item to view details");
    }
}

void InventoryScene::handleItemSelection(int slotIndex) {
    auto& inventory = GameStateManager::getInstance().getInventory();
    const auto& items = inventory.getItems();
    
    size_t actualIndex = currentPage * itemsPerPage + slotIndex;
    
    if (actualIndex < items.size()) {
        selectedSlot = slotIndex;
        selectedItem = items[actualIndex].get();
        
        // Update visual selection
        for (size_t i = 0; i < itemSlots.size(); ++i) {
            if (i == slotIndex) {
                itemSlots[i].setOutlineColor(sf::Color::White);
                itemSlots[i].setOutlineThickness(3);
            } else {
                itemSlots[i].setOutlineColor(sf::Color::Gray);
                itemSlots[i].setOutlineThickness(1);
            }
        }
        
        updateItemInfo();
        LOG_DEBUG("Selected item: {}", selectedItem->item->getName());
    }
}

void InventoryScene::useSelectedItem() {
    if (!selectedItem || !selectedItem->item) return;
    
    auto& inventory = GameStateManager::getInstance().getInventory();
    auto* player = GameStateManager::getInstance().getPlayer();
    
    if (selectedItem->item->canUse(player)) {
        std::string itemName = selectedItem->item->getName();
        
        if (inventory.useItem(selectedItem->item->getId(), player)) {
            LOG_INFO("Used item: {}", itemName);
            updateItemDisplay(); // Refresh display after use
        }
    } else {
        LOG_WARN("Cannot use item: {}", selectedItem->item->getName());
    }
}

void InventoryScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle back button
    if (backButton) {
        if (event.type == sf::Event::MouseMoved) {
            backButton->handleMouseMove(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            backButton->handleMouseClick(mousePos, event.mouseButton.button);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            backButton->handleMouseRelease(mousePos, event.mouseButton.button);
        }
    }
    
    // Handle item slot clicks
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        for (size_t i = 0; i < itemSlots.size(); ++i) {
            if (itemSlots[i].getGlobalBounds().contains(mousePos)) {
                handleItemSelection(i);
                break;
            }
        }
    }
    
    // Handle keyboard input
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            case sf::Keyboard::Enter:
                useSelectedItem();
                break;
            case sf::Keyboard::Left:
                if (currentPage > 0) {
                    currentPage--;
                    selectedSlot = -1;
                    selectedItem = nullptr;
                    updateItemDisplay();
                }
                break;
            case sf::Keyboard::Right:
                if (currentPage < totalPages - 1) {
                    currentPage++;
                    selectedSlot = -1;
                    selectedItem = nullptr;
                    updateItemDisplay();
                }
                break;
            default:
                break;
        }
    }
}

void InventoryScene::update(float deltaTime) {
    if (backButton) {
        backButton->update(deltaTime);
    }
}

void InventoryScene::render(sf::RenderWindow& window) {
    // Draw background
    window.draw(backgroundPanel);
    
    // Draw title and berry count
    window.draw(titleText);
    window.draw(berryText);
    
    // Draw item slots
    for (const auto& slot : itemSlots) {
        window.draw(slot);
    }
    
    // Draw item names and quantities
    for (const auto& name : itemNames) {
        window.draw(name);
    }
    for (const auto& quantity : itemQuantities) {
        window.draw(quantity);
    }
    
    // Draw item info
    window.draw(itemInfoText);
    
    // Draw pagination info
    if (totalPages > 1) {
        sf::Text pageText;
        pageText.setFont(FontManager::getInstance().getDefaultFont());
        pageText.setString("Page " + std::to_string(currentPage + 1) + "/" + std::to_string(totalPages) + 
                          "\nLeft/Right arrows to navigate");
        pageText.setCharacterSize(16);
        pageText.setFillColor(sf::Color::White);
        pageText.setPosition(100, 600);
        window.draw(pageText);
    }
    
    // Draw back button
    if (backButton) {
        window.draw(*backButton);
    }
}