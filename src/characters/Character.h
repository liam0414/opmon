#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include "Stats.h"
#include "DevilFruit.h"
#include "graphics/AnimatedSprite.h"

enum class CharacterType {
    Player,
    CrewMember,
    Enemy,
    NPC
};

enum class CharacterState {
    Idle,
    Walking,
    Running,
    Attacking,
    Defending,
    UsingAbility,
    Stunned,
    Dead
};

class Character : public sf::Drawable, public sf::Transformable {
protected:
    // Basic info
    std::string name;
    std::string title;
    CharacterType type;
    CharacterState state;
    
    // Stats and abilities
    std::unique_ptr<Stats> stats;
    std::unique_ptr<DevilFruit> devilFruit;
    
    // Visual representation
    std::unique_ptr<AnimatedSprite> sprite;
    sf::Vector2f velocity;
    sf::Vector2f direction;
    
    // Battle info
    long bounty;
    std::vector<std::string> crew;
    bool inBattle;
    float stunTimer;
    
    // Movement
    float moveSpeed;
    bool canMove;

public:
    Character(const std::string& characterName, CharacterType charType);
    virtual ~Character() = default;
    
    // Basic info
    const std::string& getName() const { return name; }
    const std::string& getTitle() const { return title; }
    void setTitle(const std::string& newTitle) { title = newTitle; }
    
    CharacterType getType() const { return type; }
    CharacterState getState() const { return state; }
    void setState(CharacterState newState);
    
    // Stats
    Stats& getStats() { return *stats; }
    const Stats& getStats() const { return *stats; }
    
    // Devil Fruit
    DevilFruit& getDevilFruit() { return *devilFruit; }
    const DevilFruit& getDevilFruit() const { return *devilFruit; }
    void setDevilFruit(std::unique_ptr<DevilFruit> fruit);
    bool hasDevilFruit() const;
    
    // Position and movement
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    void move(const sf::Vector2f& offset);
    void setVelocity(const sf::Vector2f& vel) { velocity = vel; }
    sf::Vector2f getVelocity() const { return velocity; }
    
    // Movement control
    void setMoveSpeed(float speed) { moveSpeed = speed; }
    float getMoveSpeed() const { return moveSpeed; }
    void setCanMove(bool canMv) { canMove = canMv; }
    bool getCanMove() const { return canMove && state != CharacterState::Stunned && state != CharacterState::Dead; }
    
    // Battle
    long getBounty() const { return bounty; }
    void setBounty(long newBounty) { bounty = newBounty; }
    void setInBattle(bool battle) { inBattle = battle; }
    bool isInBattle() const { return inBattle; }
    
    // Actions
    virtual bool useAbility(const std::string& abilityName, Character* target = nullptr);
    virtual void takeDamage(int damage, Character* attacker = nullptr);
    virtual void heal(int amount);
    virtual void stun(float duration);
    
    // Visual
    void setTexture(const std::string& textureName);
    void playAnimation(const std::string& animationName);
    sf::FloatRect getBounds() const;
    
    // Update and render
    virtual void update(float deltaTime);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    
    // Serialization
    virtual nlohmann::json toJson() const;
    virtual void fromJson(const nlohmann::json& data);
    
    // Utility
    float getDistanceTo(const Character& other) const;
    bool isAlive() const;
    bool canAct() const;
};