#include "Character.h"
#include "core/Logger.h"
#include "graphics/TextureManager.h"
#include <cmath>

Character::Character(const std::string& characterName, CharacterType charType) 
    : name(characterName), type(charType), state(CharacterState::Idle),
      bounty(0), inBattle(false), stunTimer(0), moveSpeed(100.0f), canMove(true) {
    
    stats = std::make_unique<Stats>();
    devilFruit = std::make_unique<DevilFruit>(); // No fruit by default
    sprite = std::make_unique<AnimatedSprite>();
    
    velocity = {0, 0};
    direction = {0, 1}; // Facing down by default
    
    LOG_DEBUG("Created character: {} (Type: {})", name, static_cast<int>(type));
}

void Character::setState(CharacterState newState) {
    if (state == newState) return;
    
    CharacterState oldState = state;
    state = newState;
    
    LOG_DEBUG("{} state changed: {} -> {}", name, static_cast<int>(oldState), static_cast<int>(newState));
    
    // Handle state-specific animations
    switch (state) {
        case CharacterState::Idle:
            playAnimation("idle");
            break;
        case CharacterState::Walking:
            playAnimation("walk");
            break;
        case CharacterState::Running:
            playAnimation("run");
            break;
        case CharacterState::Attacking:
            playAnimation("attack");
            break;
        case CharacterState::Defending:
            playAnimation("defend");
            break;
        case CharacterState::UsingAbility:
            playAnimation("ability");
            break;
        case CharacterState::Stunned:
            playAnimation("stunned");
            break;
        case CharacterState::Dead:
            playAnimation("dead");
            break;
    }
}

void Character::setDevilFruit(std::unique_ptr<DevilFruit> fruit) {
    devilFruit = std::move(fruit);
    LOG_INFO("{} now has Devil Fruit: {}", name, devilFruit->getName());
}

bool Character::hasDevilFruit() const {
    return devilFruit && devilFruit->getType() != DevilFruitType::None;
}

void Character::setPosition(const sf::Vector2f& pos) {
    setPosition(pos.x, pos.y);
    if (sprite) {
        sprite->setPosition(pos);
    }
}

sf::Vector2f Character::getPosition() const {
    return getTransform().transformPoint(0, 0);
}

void Character::move(const sf::Vector2f& offset) {
    sf::Transformable::move(offset);
    if (sprite) {
        sprite->move(offset);
    }
}

bool Character::useAbility(const std::string& abilityName, Character* target) {
    if (!canAct() || !hasDevilFruit()) {
        return false;
    }
    
    Ability* ability = devilFruit->getAbility(abilityName);
    if (!ability || !ability->canUse()) {
        return false;
    }
    
    // Check if we have enough devil fruit power
    int currentPower = stats->getFinalStat(StatType::DevilFruitPower);
    if (currentPower < ability->powerCost) {
        LOG_DEBUG("{} doesn't have enough power for {}", name, abilityName);
        return false;
    }
    
    // Use the ability
    ability->use();
    stats->addModifier(StatType::DevilFruitPower, -ability->powerCost);
    
    setState(CharacterState::UsingAbility);
    
    LOG_INFO("{} used {}!", name, abilityName);
    
    // Apply damage to target if applicable
    if (target && ability->baseDamage > 0) {
        int damage = ability->baseDamage + (stats->getFinalStat(StatType::Attack) / 4);
        target->takeDamage(damage, this);
    }
    
    // Add mastery points
    devilFruit->addMasteryPoints(1);
    
    return true;
}

void Character::takeDamage(int damage, Character* attacker) {
    if (!isAlive()) return;
    
    // Apply defense
    int defense = stats->getFinalStat(StatType::Defense);
    int finalDamage = std::max(1, damage - defense / 2);
    
    stats->takeDamage(finalDamage);
    
    LOG_INFO("{} took {} damage from {}! ({}/{} HP)", 
             name, finalDamage, 
             attacker ? attacker->getName() : "unknown", 
             stats->getCurrentHealth(), stats->getMaxHealth());
    
    // Check if dead
    if (!isAlive()) {
        setState(CharacterState::Dead);
        LOG_INFO("💀 {} has been defeated!", name);
        
        // Give experience to attacker
        if (attacker && attacker->getType() == CharacterType::Player) {
            int expReward = stats->getFinalStat(StatType::Level) * 25;
            attacker->getStats().addExperience(expReward);
        }
    }
}

void Character::heal(int amount) {
    if (!isAlive()) return;
    
    stats->heal(amount);
    LOG_INFO("{} healed for {} HP! ({}/{})", 
             name, amount, stats->getCurrentHealth(), stats->getMaxHealth());
}

void Character::stun(float duration) {
    stunTimer = duration;
    setState(CharacterState::Stunned);
    LOG_INFO("{} is stunned for {:.1f} seconds!", name, duration);
}

void Character::setTexture(const std::string& textureName) {
    if (sprite) {
        auto& textureManager = TextureManager::getInstance();
        if (textureManager.hasTexture(textureName)) {
            sprite->setTexture(textureManager.getTexture(textureName));
            LOG_DEBUG("Set texture '{}' for character '{}'", textureName, name);
        } else {
            LOG_WARN("Texture '{}' not found for character '{}'", textureName, name);
        }
    }
}

void Character::playAnimation(const std::string& animationName) {
    if (sprite) {
        sprite->playAnimation(animationName);
    }
}

sf::FloatRect Character::getBounds() const {
    if (sprite) {
        return sprite->getGlobalBounds();
    }
    
    // Default bounds if no sprite
    sf::Vector2f pos = getPosition();
    return sf::FloatRect(pos.x - 16, pos.y - 16, 32, 32);
}

void Character::update(float deltaTime) {
    // Update stats regeneration
    if (isAlive()) {
        // Regenerate devil fruit power over time
        if (hasDevilFruit()) {
            int currentPower = stats->getFinalStat(StatType::DevilFruitPower);
            int maxPower = stats->getFinalStat(StatType::Level) * 10; // Max power scales with level
            
            if (currentPower < maxPower) {
                int regen = static_cast<int>(5.0f * deltaTime); // 5 power per second
                stats->addModifier(StatType::DevilFruitPower, regen);
            }
        }
        
        // Handle stun timer
        if (stunTimer > 0) {
            stunTimer -= deltaTime;
            if (stunTimer <= 0) {
                setState(CharacterState::Idle);
            }
        }
        
        // Update movement based on velocity
        if (getCanMove() && (velocity.x != 0 || velocity.y != 0)) {
            sf::Vector2f movement = velocity * deltaTime;
            move(movement);
            
            // Update direction
            float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            if (length > 0) {
                direction = velocity / length;
            }
            
            // Set appropriate movement state
            if (length > moveSpeed * 0.8f) {
                setState(CharacterState::Running);
            } else if (length > 0.1f) {
                setState(CharacterState::Walking);
            } else {
                setState(CharacterState::Idle);
            }
            
            // Handle sprite flipping based on direction
            if (sprite) {
                sprite->setFlip(direction.x < 0, false);
            }
        } else if (state == CharacterState::Walking || state == CharacterState::Running) {
            setState(CharacterState::Idle);
        }
    }
    
    // Update devil fruit abilities
    if (devilFruit) {
        devilFruit->update(deltaTime);
    }
    
    // Update sprite animation
    if (sprite) {
        sprite->update(deltaTime);
    }
}

void Character::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (sprite) {
        states.transform *= getTransform();
        target.draw(*sprite, states);
    }
}

nlohmann::json Character::toJson() const {
    nlohmann::json data = {
        {"name", name},
        {"title", title},
        {"type", static_cast<int>(type)},
        {"bounty", bounty},
        {"moveSpeed", moveSpeed},
        {"position", {getPosition().x, getPosition().y}},
        {"velocity", {velocity.x, velocity.y}},
        {"direction", {direction.x, direction.y}}
    };
    
    if (stats) {
        data["stats"] = stats->toJson();
    }
    
    if (devilFruit) {
        data["devilFruit"] = devilFruit->toJson();
    }
    
    return data;
}

void Character::fromJson(const nlohmann::json& data) {
    name = data.value("name", "Unknown");
    title = data.value("title", "");
    type = static_cast<CharacterType>(data.value("type", 0));
    bounty = data.value("bounty", 0L);
    moveSpeed = data.value("moveSpeed", 100.0f);
    
    if (data.contains("position")) {
        auto pos = data["position"];
        setPosition(sf::Vector2f(pos[0], pos[1]));
    }
    
    if (data.contains("velocity")) {
        auto vel = data["velocity"];
        velocity = sf::Vector2f(vel[0], vel[1]);
    }
    
    if (data.contains("direction")) {
        auto dir = data["direction"];
        direction = sf::Vector2f(dir[0], dir[1]);
    }
    
    if (data.contains("stats") && stats) {
        stats->fromJson(data["stats"]);
    }
    
    if (data.contains("devilFruit") && devilFruit) {
        devilFruit->fromJson(data["devilFruit"]);
    }
}

float Character::getDistanceTo(const Character& other) const {
    sf::Vector2f pos1 = getPosition();
    sf::Vector2f pos2 = other.getPosition();
    
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    
    return std::sqrt(dx * dx + dy * dy);
}

bool Character::isAlive() const {
    return stats && stats->isAlive();
}

bool Character::canAct() const {
    return isAlive() && state != CharacterState::Stunned && state != CharacterState::Dead;
}