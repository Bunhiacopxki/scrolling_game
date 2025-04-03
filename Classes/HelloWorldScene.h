#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "Player.h"
#include "Monster.h"
#include "Chest.h"
#include "Boss.h"
#include "MenuScene.h"
#include "Board.h"
#include <cstdlib>

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene *createScene();

    bool OnPhysicsContact(cocos2d::PhysicsContact &contact);
    virtual bool init();
    void update(float delta);

    // a selector callback
    void menuCloseCallback(cocos2d::Ref *pSender);

    void initMusic();
    cocos2d::TMXObjectGroup *initObject(cocos2d::TMXTiledMap *tileMap);
    void initKeyboardListener();
    void initGameSchedule(TMXTiledMap *tileMap, Player *player, const Size &visibleSize);
    void initMap2(TMXTiledMap *tileMap);
    bool initMap3(TMXTiledMap *tileMap);
    void initMap4(TMXTiledMap* tileMap);
    void randomAttack(float dt);
    void scheduleNextAttack();
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

private:
    cocos2d::PhysicsWorld *world;
    void setPhysicWorld(cocos2d::PhysicsWorld *m_world)
    {
        world = m_world;
    }
    Player *player;
    Monster *monster1;
    // bool isOnGround;
    std::vector<Chest *> chests;
    Chest *treasure;
    Boss *boss;
    std::vector<Monster *> monsters; // Khai báo biến monsters
    bool _isLeftPressed = false;
    bool _isRightPressed = false;
    bool _isUpPressed = false;
    bool _isDownPressed = false;
    bool _isLeftBlocked = false;
    bool _isRightBlocked = false;
    float _lastCorrectionTime;
    float _correctionInterval;
    bool flag = false;
    Camera *camera = nullptr;
    TMXTiledMap *tileMap = nullptr;
    bool isOnGround = false;
    Label *goldLabel = nullptr;
    Node *uiLayer = nullptr;
    //Boss *boss;
    Board *board;
    Board *live;
    bool BossAttack = false;
    ProgressTimer* bossHealthBar = nullptr;

};

#endif // __HELLOWORLD_SCENE_H__
