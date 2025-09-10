#include "ShopScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "game/GameStateManager.h"

ShopScene::ShopScene(const std::string& name, const std::string& keeper) 
    : shopName(name), shopkeeper(keeper), currentMode(ShopMode::Buy), 
      selectedItemIndex(-1), quantityToTrade(1), currentPage(0), itemsPerPage(12) {
    
    setupUI();
    createDefaultShop();
}

void ShopScene::onEnter() {
    LOG_INFO("Opened shop: {} (Shopkeeper: {})", shopName, shopkeeper);
    updateItemDisplay();
    updatePlayerInfo();
}

void ShopScene::onExit() {
    LOG_INFO("Closed shop: {}", shopName);
}

void ShopScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Background
    backgroundPanel.setSize(sf::Vector2f(980, 700));
    backgroundPanel.setPosition(22, 34);
    backgroundPanel.setFillColor(sf::Color(40, 35, 30, 240));
    backgroundPanel.setOutlineColor(sf::Color(150, 100, 50));
    backgroundPanel.setOutlineThickness(3);
    
    // Info panel
    infoPanel.setSize(sf::Vector2f(300, 400));
    infoPanel.setPosition(680, 150);
    infoPanel.setFillColor(sf::Color(30, 25, 20, 200));
    infoPanel.setOutlineColor(sf::Color::Gray);
    infoPanel.setOutlineThickness(2);
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString(shopName);
    titleText.setCharacterSize(32);
    titleText.setFillColor(sf::Color(255, 215, 0)); // Gold
    titleText.setPosition(50, 50);
    
    // Shopkeeper
    shopkeeperText.setFont(fontManager.getDefaultFont());
    shopkeeperText.setString("Shopkeeper: " + shopkeeper);
    shopkeeperText.setCharacterSize(20);
    shopkeeperText.setFillColor(sf::Color::White);
    shopkeeperText.setPosition(50, 90);
    
    // Player berry display
    playerBerryText.setFont(fontManager.getDefaultFont());
    playerBerryText.setCharacterSize(24);
    playerBerryText.setFillColor(sf::Color(255, 215, 0));
    playerBerryText.setPosition(50, 120);
    
    // Item info
    itemInfoText.setFont(fontManager.getDefaultFont());
    itemInfoText.setCharacterSize(16);
    itemInfoText.setFillColor(sf::Color::White);
    itemInfoText.setPosition(690, 160);
    
    // Mode buttons
    buyModeButton = std::make_unique<Button>("BUY", fontManager.getDefaultFont());
    buyModeButton->setBounds(sf::FloatRect(50, 680, 100, 40));
    buyModeButton->setColors(sf::Color(0, 150, 0, 200), sf::Color(0, 180, 0, 220),
                            sf::Color(0, 120, 0, 240), sf::Color::Gray);
    buyModeButton->setOnClick([this]() { switchMode(ShopMode::Buy); });
    
    sellModeButton = std::make_unique<Button>("SELL", fontManager.getDefaultFont());
    sellModeButton->setBounds(sf::FloatRect(170, 680, 100, 40));
    sellModeButton->setColors(sf::Color(150, 150, 0, 200), sf::Color(180, 180, 0, 220),
                             sf::Color(120, 120, 0, 240), sf::Color::Gray);
    sellModeButton->setOnClick([this]() { switchMode(ShopMode::Sell); });
    
    // Transaction buttons
    buyButton = std::make_unique<Button>("Buy Item", fontManager.getDefaultFont());
    buyButton->setBounds(sf::FloatRect(400, 680, 120, 40));
    buyButton->setColors(sf::Color(50, 100, 150, 200), sf::Color(70, 120, 170, 220),
                        sf::Color(30, 80, 130, 240), sf::Color::Gray);
    buyButton->setOnClick([this]() { buySelectedItem(); });
    buyButton->setInteractive(false);
    
    sellButton = std::make_unique<Button>("Sell Item", fontManager.getDefaultFont());
    sellButton->setBounds(sf::FloatRect(400, 680, 120, 40));
    sellButton->setColors(sf::Color(150, 100, 50, 200), sf::Color(170, 120, 70, 220),
                         sf::Color(130, 80, 30, 240), sf::Color::Gray);
    sellButton->setOnClick([this]() { sellSelectedItem(); });
    sellButton->setInteractive(false);
    
    // Exit button
    exitButton = std::make_unique<Button>("Leave", fontManager.getDefaultFont());
    exitButton->setBounds(sf::FloatRect(880, 680, 100, 40));
    exitButton->setColors(sf::Color(100, 50, 50, 200), sf::Color(120, 70, 70, 220),
                         sf::Color(80, 30, 30, 240), sf::Color::Gray);
    exitButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
    
    // Create item slots (3x4 grid)
    float slotSize = 80.0f;
    float slotSpacing = 90.0f;
    float startX = 60.0f;
    float startY = 180.0f;
    
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            int slotIndex = row * 3 + col;
            
            // Item slot
            sf::RectangleShape slot;
            slot.setSize(sf::Vector2f(slotSize, slotSize));
            slot.setPosition(startX + col * slotSpacing, startY + row * slotSpacing);
            slot.setFillColor(sf::Color(60, 50, 40, 200));
            slot.setOutlineColor(sf::Color::Gray);
            slot.setOutlineThickness(1);
            itemSlots.push_back(slot);
            
            // Item name
            sf::Text nameText;
            nameText.setFont(fontManager.getDefaultFont());
            nameText.setCharacterSize(12);
            nameText.setFillColor(sf::Color::White);
            nameText.setPosition(startX + col * slotSpacing + 5, startY + row * slotSpacing + slotSize + 5);
            itemNames.push_back(nameText);
            
            // Price
            sf::Text priceText;
            priceText.setFont(fontManager.getDefaultFont());
            priceText.setCharacterSize(12);
            priceText.setFillColor(sf::Color(255, 215, 0));
            priceText.setPosition(startX + col * slotSpacing + 5, startY + row * slotSpacing + slotSize + 25);
            itemPrices.push_back(priceText);
            
            // Stock
            sf::Text stockText;
            stockText.setFont(fontManager.getDefaultFont());
            stockText.setCharacterSize(10);
            stockText.setFillColor(sf::Color::Cyan);
            stockText.setPosition(startX + col * slotSpacing + slotSize - 20, startY + row * slotSpacing + 5);
            itemStock.push_back(stockText);
        }
    }
    
    // Start in buy mode
    switchMode(ShopMode::Buy);
}

void ShopScene::updateItemDisplay() {
    // Clear current display
    for (size_t i = 0; i < itemSlots.size(); ++i) {
        itemSlots[i].setFillColor(sf::Color(60, 50, 40, 200));
        itemSlots[i].setOutlineColor(sf::Color::Gray);
        itemSlots[i].setOutlineThickness(1);
        itemNames[i].setString("");
        itemPrices[i].setString("");
        itemStock[i].setString("");
    }
    
    if (currentMode == ShopMode::Buy) {
        // Display shop inventory
        size_t startIndex = currentPage * itemsPerPage;
        size_t endIndex = std::min(startIndex + itemsPerPage, shopInventory.size());
        
        for (size_t i = startIndex; i < endIndex; ++i) {
            size_t slotIndex = i - startIndex;
            if (slotIndex >= itemSlots.size()) break;
            
            const auto& shopItem = shopInventory[i];
            if (shopItem && shopItem->item && shopItem->available) {
                // Set item name
                std::string displayName = shopItem->item->getName();
                if (displayName.length() > 10) {
                    displayName = displayName.substr(0, 10) + "...";
                }
                itemNames[slotIndex].setString(displayName);
                
                // Set price
                itemPrices[slotIndex].setString(std::to_string(shopItem->price) + "฿");
                
                // Set stock
                if (shopItem->stock >= 0) {
                    itemStock[slotIndex].setString("x" + std::to_string(shopItem->stock));
                } else {
                    itemStock[slotIndex].setString("∞");
                }
                
                // Color based on rarity
                sf::Color slotColor;
                switch (shopItem->item->getRarity()) {
                    case ItemRarity::Common:    slotColor = sf::Color(100, 80, 60, 200); break;
                    case ItemRarity::Uncommon:  slotColor = sf::Color(60, 120, 60, 200); break;
                    case ItemRarity::Rare:      slotColor = sf::Color(60, 80, 160, 200); break;
                    case ItemRarity::Epic:      slotColor = sf::Color(160, 60, 160, 200); break;
                    case ItemRarity::Legendary: slotColor = sf::Color(255, 165, 0, 200); break;
                }
                itemSlots[slotIndex].setFillColor(slotColor);
            }
        }
    } else {
        // Display player inventory for selling
        auto& inventory = GameStateManager::getInstance().getInventory();
        const auto& playerItems = inventory.getItems();
        
        size_t startIndex = currentPage * itemsPerPage;
        size_t endIndex = std::min(startIndex + itemsPerPage, playerItems.size());
        
        for (size_t i = startIndex; i < endIndex; ++i) {
            size_t slotIndex = i - startIndex;
            if (slotIndex >= itemSlots.size()) break;
            
            const auto& itemStack = playerItems[i];
            if (itemStack && itemStack->item) {
                // Set item name
                std::string displayName = itemStack->item->getName();
                if (displayName.length() > 10) {
                    displayName = displayName.substr(0, 10) + "...";
                }
                itemNames[slotIndex].setString(displayName);
                
                // Set sell price (50% of value)
                int sellPrice = itemStack->item->getValue() / 2;
                itemPrices[slotIndex].setString(std::to_string(sellPrice) + "฿");
                
                // Set quantity
                itemStock[slotIndex].setString("x" + std::to_string(itemStack->quantity));
                
                // Color based on rarity
                sf::Color slotColor;
                switch (itemStack->item->getRarity()) {
                    case ItemRarity::Common:    slotColor = sf::Color(100, 80, 60, 200); break;
                    case ItemRarity::Uncommon:  slotColor = sf::Color(60, 120, 60, 200); break;
                    case ItemRarity::Rare:      slotColor = sf::Color(60, 80, 160, 200); break;
                    case ItemRarity::Epic:      slotColor = sf::Color(160, 60, 160, 200); break;
                    case ItemRarity::Legendary: slotColor = sf::Color(255, 165, 0, 200); break;
                }
                itemSlots[slotIndex].setFillColor(slotColor);
            }
        }
    }
    
    showItemInfo();
}

void ShopScene::updatePlayerInfo() {
    auto& inventory = GameStateManager::getInstance().getInventory();
    playerBerryText.setString("Berry: " + std::to_string(inventory.getBerry()) + " ฿");
}

void ShopScene::selectItem(int index) {
    // Deselect previous item
    if (selectedItemIndex >= 0 && selectedItemIndex < static_cast<int>(itemSlots.size())) {
        itemSlots[selectedItemIndex].setOutlineThickness(1);
    }
    
    selectedItemIndex = index;
    
    if (selectedItemIndex >= 0 && selectedItemIndex < static_cast<int>(itemSlots.size())) {
        // Highlight selected item
        itemSlots[selectedItemIndex].setOutlineColor(sf::Color::White);
        itemSlots[selectedItemIndex].setOutlineThickness(3);
        
        // Enable transaction button
        if (currentMode == ShopMode::Buy) {
            buyButton->setInteractive(true);
            buyButton->setState(UIState::Normal);
        } else {
            sellButton->setInteractive(true);
            sellButton->setState(UIState::Normal);
        }
        
        showItemInfo();
    }
}

void ShopScene::buySelectedItem() {
    if (selectedItemIndex < 0 || currentMode != ShopMode::Buy) return;
    
    size_t shopIndex = currentPage * itemsPerPage + selectedItemIndex;
    if (shopIndex >= shopInventory.size()) return;
    
    const auto& shopItem = shopInventory[shopIndex];
    if (!shopItem || !shopItem->item || !shopItem->available) return;
    
    auto& inventory = GameStateManager::getInstance().getInventory();
    
    // Check if player has enough money
    if (inventory.getBerry() < shopItem->price) {
        LOG_WARN("Not enough money to buy {}: {} Berry needed, {} available", 
                shopItem->item->getName(), shopItem->price, inventory.getBerry());
        return;
    }
    
    // Check stock
    if (shopItem->stock == 0) {
        LOG_WARN("Item {} is out of stock", shopItem->item->getName());
        return;
    }
    
    // Create copy of item for player
    // Note: This is a simplified version - in a full implementation, you'd want proper item cloning
    auto itemCopy = std::make_unique<Item>(shopItem->item->getId(), shopItem->item->getName(), 
                                          shopItem->item->getType(), shopItem->item->getRarity());
    itemCopy->setValue(shopItem->item->getValue());
    itemCopy->setDescription(shopItem->item->getDescription());
    
    // Add to player inventory
    if (inventory.addItem(std::move(itemCopy), quantityToTrade)) {
        // Deduct money
        inventory.spendBerry(shopItem->price);
        
        // Update shop stock
        if (shopItem->stock > 0) {
            shopItem->stock--;
            if (shopItem->stock == 0) {
                shopItem->available = false;
            }
        }
        
        LOG_INFO("Purchased {} for {} Berry", shopItem->item->getName(), shopItem->price);
        
        updateItemDisplay();
        updatePlayerInfo();
    } else {
        LOG_WARN("Inventory full - cannot purchase {}", shopItem->item->getName());
    }
}

void ShopScene::sellSelectedItem() {
    if (selectedItemIndex < 0 || currentMode != ShopMode::Sell) return;
    
    auto& inventory = GameStateManager::getInstance().getInventory();
    const auto& playerItems = inventory.getItems();
    
    size_t itemIndex = currentPage * itemsPerPage + selectedItemIndex;
    if (itemIndex >= playerItems.size()) return;
    
    const auto& itemStack = playerItems[itemIndex];
    if (!itemStack || !itemStack->item) return;
    
    // Calculate sell price (50% of item value)
    int sellPrice = itemStack->item->getValue() / 2;
    
    // Remove item from inventory
    if (inventory.removeItem(itemStack->item->getId(), quantityToTrade)) {
        // Give money to player
        inventory.addBerry(sellPrice);
        
        LOG_INFO("Sold {} for {} Berry", itemStack->item->getName(), sellPrice);
        
        updateItemDisplay();
        updatePlayerInfo();
        
        // Deselect if item stack is empty
        if (itemStack->quantity <= 0) {
            selectedItemIndex = -1;
            sellButton->setInteractive(false);
            sellButton->setState(UIState::Disabled);
        }
    }
}

void ShopScene::switchMode(ShopMode mode) {
    currentMode = mode;
    selectedItemIndex = -1;
    currentPage = 0;
    
    // Update button states
    if (mode == ShopMode::Buy) {
        buyModeButton->setState(UIState::Pressed);
        sellModeButton->setState(UIState::Normal);
        buyButton->setVisible(true);
        sellButton->setVisible(false);
    } else {
        buyModeButton->setState(UIState::Normal);
        sellModeButton->setState(UIState::Pressed);
        buyButton->setVisible(false);
        sellButton->setVisible(true);
    }
    
    // Disable transaction buttons until item is selected
    buyButton->setInteractive(false);
    sellButton->setInteractive(false);
    
    updateItemDisplay();
}

void ShopScene::showItemInfo() {
    std::string info = "Select an item to view details";
    
    if (selectedItemIndex >= 0) {
        if (currentMode == ShopMode::Buy) {
            size_t shopIndex = currentPage * itemsPerPage + selectedItemIndex;
            if (shopIndex < shopInventory.size()) {
                const auto& shopItem = shopInventory[shopIndex];
                if (shopItem && shopItem->item) {
                    info = "=== " + shopItem->item->getName() + " ===\n\n";
                    info += shopItem->item->getDescription() + "\n\n";
                    info += "Type: ";
                    switch (shopItem->item->getType()) {
                        case ItemType::Consumable: info += "Consumable"; break;
                        case ItemType::Weapon: info += "Weapon"; break;
                        case ItemType::Armor: info += "Armor"; break;
                        case ItemType::Accessory: info += "Accessory"; break;
                        case ItemType::KeyItem: info += "Key Item"; break;
                        case ItemType::Treasure: info += "Treasure"; break;
                        case ItemType::Material: info += "Material"; break;
                    }
                    info += "\nRarity: ";
switch (shopItem->item->getRarity()) {
                        case ItemRarity::Common: info += "Common"; break;
                        case ItemRarity::Uncommon: info += "Uncommon"; break;
                        case ItemRarity::Rare: info += "Rare"; break;
                        case ItemRarity::Epic: info += "Epic"; break;
                        case ItemRarity::Legendary: info += "Legendary"; break;
                    }
                    info += "\nPrice: " + std::to_string(shopItem->price) + " Berry";
                    if (shopItem->stock >= 0) {
                        info += "\nStock: " + std::to_string(shopItem->stock);
                    } else {
                        info += "\nStock: Unlimited";
                    }
                }
            }
        } else {
            auto& inventory = GameStateManager::getInstance().getInventory();
            const auto& playerItems = inventory.getItems();
            size_t itemIndex = currentPage * itemsPerPage + selectedItemIndex;
            
            if (itemIndex < playerItems.size()) {
                const auto& itemStack = playerItems[itemIndex];
                if (itemStack && itemStack->item) {
                    info = "=== " + itemStack->item->getName() + " ===\n\n";
                    info += itemStack->item->getDescription() + "\n\n";
                    info += "Sell Price: " + std::to_string(itemStack->item->getValue() / 2) + " Berry";
                    info += "\nOwned: " + std::to_string(itemStack->quantity);
                }
            }
        }
    }
    
    itemInfoText.setString(info);
}

void ShopScene::addShopItem(std::unique_ptr<Item> item, int price, int stock) {
    shopInventory.push_back(std::make_unique<ShopItem>(std::move(item), price, stock));
}

void ShopScene::createDefaultShop() {
    // Create some basic items for the shop
    auto healthPotion = std::make_unique<Item>("health_potion", "Health Potion", ItemType::Consumable);
    healthPotion->setDescription("Restores 50 HP when consumed");
    healthPotion->setValue(100);
    addShopItem(std::move(healthPotion), 50, 10);
    
    auto energyDrink = std::make_unique<Item>("energy_drink", "Energy Drink", ItemType::Consumable);
    energyDrink->setDescription("Restores stamina and increases speed temporarily");
    energyDrink->setValue(150);
    addShopItem(std::move(energyDrink), 75, 5);
    
    auto basicSword = std::make_unique<Item>("basic_sword", "Basic Sword", ItemType::Weapon, ItemRarity::Common);
    basicSword->setDescription("A simple but reliable blade");
    basicSword->setValue(500);
    addShopItem(std::move(basicSword), 400, 3);
}

void ShopScene::setShopInventory(std::vector<std::unique_ptr<ShopItem>> inventory) {
    shopInventory = std::move(inventory);
}

void ShopScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle buttons
    std::vector<Button*> buttons = {
        buyModeButton.get(), sellModeButton.get(), 
        buyButton.get(), sellButton.get(), exitButton.get()
    };
    
    for (auto* button : buttons) {
        if (button && button->isVisible()) {
            if (event.type == sf::Event::MouseMoved) {
                button->handleMouseMove(mousePos);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                button->handleMouseClick(mousePos, event.mouseButton.button);
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                button->handleMouseRelease(mousePos, event.mouseButton.button);
            }
        }
    }
    
    // Handle item slot clicks
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        for (size_t i = 0; i < itemSlots.size(); ++i) {
            if (itemSlots[i].getGlobalBounds().contains(mousePos)) {
                selectItem(i);
                break;
            }
        }
    }
    
    // Keyboard shortcuts
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            case sf::Keyboard::Tab:
                switchMode(currentMode == ShopMode::Buy ? ShopMode::Sell : ShopMode::Buy);
                break;
            case sf::Keyboard::Enter:
                if (selectedItemIndex >= 0) {
                    if (currentMode == ShopMode::Buy) {
                        buySelectedItem();
                    } else {
                        sellSelectedItem();
                    }
                }
                break;
            case sf::Keyboard::Left:
                if (currentPage > 0) {
                    currentPage--;
                    updateItemDisplay();
                }
                break;
            case sf::Keyboard::Right:
                // Check if there are more items to show
                size_t totalItems = (currentMode == ShopMode::Buy) ? 
                                   shopInventory.size() : 
                                   GameStateManager::getInstance().getInventory().getItems().size();
                if ((currentPage + 1) * itemsPerPage < totalItems) {
                    currentPage++;
                    updateItemDisplay();
                }
                break;
            default:
                break;
        }
    }
}

void ShopScene::update(float deltaTime) {
    // Update buttons
    buyModeButton->update(deltaTime);
    sellModeButton->update(deltaTime);
    if (buyButton->isVisible()) buyButton->update(deltaTime);
    if (sellButton->isVisible()) sellButton->update(deltaTime);
    exitButton->update(deltaTime);
}

void ShopScene::render(sf::RenderWindow& window) {
    // Clear background
    window.clear(sf::Color(15, 20, 25));
    
    // Draw panels
    window.draw(backgroundPanel);
    window.draw(infoPanel);
    
    // Draw title and info
    window.draw(titleText);
    window.draw(shopkeeperText);
    window.draw(playerBerryText);
    window.draw(itemInfoText);
    
    // Draw item slots
    for (const auto& slot : itemSlots) {
        window.draw(slot);
    }
    for (const auto& name : itemNames) {
        window.draw(name);
    }
    for (const auto& price : itemPrices) {
        window.draw(price);
    }
    for (const auto& stock : itemStock) {
        window.draw(stock);
    }
    
    // Draw mode indicator
    sf::Text modeText;
    modeText.setFont(FontManager::getInstance().getDefaultFont());
    modeText.setString(currentMode == ShopMode::Buy ? "BUY MODE" : "SELL MODE");
    modeText.setCharacterSize(24);
    modeText.setFillColor(currentMode == ShopMode::Buy ? sf::Color::Green : sf::Color::Yellow);
    modeText.setPosition(300, 680);
    window.draw(modeText);
    
    // Draw buttons
    window.draw(*buyModeButton);
    window.draw(*sellModeButton);
    if (buyButton->isVisible()) window.draw(*buyButton);
    if (sellButton->isVisible()) window.draw(*sellButton);
    window.draw(*exitButton);
    
    // Draw instructions
    sf::Text instructionText;
    instructionText.setFont(FontManager::getInstance().getDefaultFont());
    instructionText.setString("Tab to switch modes | Enter to buy/sell | Left/Right for pages | ESC to exit");
    instructionText.setCharacterSize(12);
    instructionText.setFillColor(sf::Color::Gray);
    instructionText.setPosition(50, 740);
    window.draw(instructionText);
}

std::unique_ptr<ShopScene> ShopScene::createGeneralStore() {
    auto shop = std::make_unique<ShopScene>("General Store", "Merchant Bob");
    
    // Add variety of items
    auto map = std::make_unique<Item>("world_map", "World Map", ItemType::KeyItem, ItemRarity::Uncommon);
    map->setDescription("Shows nearby islands and safe routes");
    map->setValue(1000);
    shop->addShopItem(std::move(map), 800, 1);
    
    auto compass = std::make_unique<Item>("compass", "Compass", ItemType::Accessory, ItemRarity::Common);
    compass->setDescription("Never lose your way again");
    compass->setValue(200);
    shop->addShopItem(std::move(compass), 150, 5);
    
    return shop;
}

std::unique_ptr<ShopScene> ShopScene::createWeaponShop() {
    auto shop = std::make_unique<ShopScene>("Weapon Emporium", "Blacksmith Jack");
    
    auto cutlass = std::make_unique<Item>("cutlass", "Pirate Cutlass", ItemType::Weapon, ItemRarity::Common);
    cutlass->setDescription("A classic pirate sword with a curved blade");
    cutlass->setValue(800);
    shop->addShopItem(std::move(cutlass), 600, 3);
    
    auto flintlock = std::make_unique<Item>("flintlock", "Flintlock Pistol", ItemType::Weapon, ItemRarity::Uncommon);
    flintlock->setDescription("A reliable firearm for long-range combat");
    flintlock->setValue(1200);
    shop->addShopItem(std::move(flintlock), 1000, 2);
    
    return shop;
}

std::unique_ptr<ShopScene> ShopScene::createFoodShop() {
    auto shop = std::make_unique<ShopScene>("Sanji's Kitchen", "Chef Sanji");
    
    auto meat = std::make_unique<Item>("grilled_meat", "Grilled Meat", ItemType::Consumable, ItemRarity::Common);
    meat->setDescription("Delicious grilled meat that restores health and boosts attack");
    meat->setValue(150);
    shop->addShopItem(std::move(meat), 100, 20);
    
    auto seafood = std::make_unique<Item>("sea_king_steak", "Sea King Steak", ItemType::Consumable, ItemRarity::Rare);
    seafood->setDescription("Premium meat from a Sea King. Greatly boosts all stats temporarily");
    seafood->setValue(2000);
    shop->addShopItem(std::move(seafood), 1500, 1);
    
    return shop;
}