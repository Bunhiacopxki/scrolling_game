﻿#include "HelloWorldScene.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;

Scene *HelloWorld::createScene()
{
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_NONE);
    auto layer = HelloWorld::create();
    layer->setPhysicWorld(scene->getPhysicsWorld());
    scene->getPhysicsWorld()->setGravity(Vec2(0, -300));
    scene->addChild(layer);
    return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char *filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

bool HelloWorld::init()
{
    if (!Layer::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Khởi tạo nhạc nền
    initMusic();

    // Tạo tile map từ file TMX
    tileMap = TMXTiledMap::create("tile2/map4.tmx");
    if (tileMap == nullptr)
    {
        problemLoading("'tile2/map4.tmx'");
        return false;
    }
    tileMap->setPosition(Vec2(0, 0));
    tileMap->setCameraMask((unsigned short)CameraFlag::DEFAULT);
    this->addChild(tileMap, 0);

    // Tạo object group từ tile map (nếu có)
    auto objectGroup = initObject(tileMap);
    if (objectGroup == nullptr)
    {
        problemLoading("objectGroup");
        return false;
    }
    initMap2(tileMap);
    if (!initMap3(tileMap))
        return false;
    initMap4(tileMap);

    // --- Kết thúc phần MAP ---

    // --- Phần nhân vật từ code đầu tiên ---
    // Giả sử bạn đã có lớp Player riêng với hàm tạo đã tích hợp physics body
    player = Player::createPlayer();
    player->setPosition(Vec2(400, 2000));
    this->addChild(player);
    this->addChild(player->fire);
    
    

    uiLayer = Node::create();
    this->addChild(uiLayer, 200); // UI luôn nằm trên top
    this->board = Board::create(true);
    this->live = Board::create(false);
    uiLayer->addChild(board);
    uiLayer->addChild(live);
    // Tạo Label hiển thị gold
    goldLabel = Label::createWithTTF("Gold: 0", "fonts/Marker Felt.ttf", 50);
    goldLabel->setAnchorPoint(Vec2(0, 0)); // Căn phải trên
    goldLabel->setPosition(visibleSize.width - 170, visibleSize.height - 60);
    goldLabel->setTextColor(Color4B(255, 215, 0, 255)); // Màu vàng
    uiLayer->addChild(goldLabel, 10);

    // Tạo Follow action
    auto followAction = Follow::create(player, Rect::ZERO);
    this->runAction(followAction);

    initKeyboardListener();
    initGameSchedule(tileMap, player, visibleSize);

    // Thiết lập sự kiện va chạm
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(HelloWorld::OnPhysicsContact, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

    this->scheduleUpdate();

    return true;
}

// Hàm xử lý nhạc nền (từ code thứ hai)
void HelloWorld::initMusic()
{
    AudioEngine::stopAll();
    // AudioEngine::play2d("haha/Sealed Vessel.mp3", true, 0.5f);
}

// Hàm khởi tạo object từ tile map (từ code thứ hai)
TMXObjectGroup *HelloWorld::initObject(TMXTiledMap *tileMap)
{
    // Sau khi khởi tạo tileMap trong init()
    // pushContainer = Node::create();
    // pushContainer->setPosition(Vec2::ZERO); // Vị trí ban đầu
    // this->addChild(pushContainer, 2);

    auto objectGroup = tileMap->getObjectGroup("Object Layer");
    if (objectGroup == nullptr)
    {
        log("Lỗi: Không tìm thấy object group");
        return nullptr;
    }

    auto objects = objectGroup->getObjects();
    for (auto &obj : objects)
    {
        ValueMap object = obj.asValueMap();

        std::string collidable = object["collidable"].asString();
        std::string poison = object["poison"].asString();
        std::string spike = object["spike"].asString();
        std::string hit = object["hit"].asString();
        std::string fly = object["fly"].asString();
        std::string take = object["take"].asString();
        std::string dead = object["dead"].asString();
        std::string push = object["push"].asString();
        std::string climb = object["climb"].asString();

        float x = object["x"].asFloat();
        float y = object["y"].asFloat();
        float width = object["width"].asFloat();
        float height = object["height"].asFloat();

        auto objectNode = Node::create();
        objectNode->setPosition(Vec2(x + width / 2, y + height / 2));

        auto physicsBody = PhysicsBody::createBox(Size(width, height));
        physicsBody->setDynamic(false);

        /*auto pushContainer = Node::create();
        this->addChild(pushContainer);*/

        int categoryBitmask = 0x00;
        int collisionBitmask = 0x01;

        if (collidable == "true")
            categoryBitmask |= 0x02;
        if (fly == "true")
            categoryBitmask |= 0x02;
        if (dead == "true")
            categoryBitmask |= 0x04;
        if (poison == "true")
            categoryBitmask |= 0x04;
        if (spike == "true")
            categoryBitmask |= 0x04;
        if (hit == "true")
            categoryBitmask |= 0x08;
        /*
        else {
            tileMap->addChild(objectNode);
        }*/
        if (climb == "true")
            categoryBitmask |= 0x20;

        physicsBody->setCategoryBitmask(categoryBitmask);
        // physicsBody->setCollisionBitmask(collisionBitmask);
        physicsBody->setContactTestBitmask(0x01);
        objectNode->setPhysicsBody(physicsBody);
        tileMap->addChild(objectNode);
    }
    return objectGroup;
}

bool HelloWorld::OnPhysicsContact(cocos2d::PhysicsContact &contact)
{
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();

    // Lấy category bitmask của hai đối tượng
    int categoryA = bodyA->getCategoryBitmask();
    int categoryB = bodyB->getCategoryBitmask();
    if ((categoryA == 0x100 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x100) || (categoryA == 0x101 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x101) || (categoryA == 0x108 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x108))
    {
        CCLOG("Player đã va chạm với Monster!");
        // Xử lý logic khi va chạm, ví dụ trừ máu player hoặc xóa monster
        if (player)
        {
            player->takeDamage(1);
        }
        return true;
    }

    
    if ((categoryA == 0x04 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x04))
    {
        CCLOG("Player đã dính bẫy!");
        // Xử lý logic khi va chạm, ví dụ trừ máu player hoặc xóa monster
        if (player)
        {
            int dame = 1;
            player->takeDamage(dame);
            // Đặt lịch cập nhật vị trí sau 0.5 giây
            this->scheduleOnce([this](float dt)
                               {
                auto moveUp = MoveBy::create(0.3f, Vec2(0, 150)); // Di chuyển lên 100px trong 0.5 giây
                player->runAction(moveUp); }, 0.2f, "update_player_position");
        }
        return true;
    }

    if ((categoryA == 0x101 && categoryB == 0x80) || (categoryA == 0x80 && categoryB == 0x101) || (categoryA == 0x100 && categoryB == 0x80) || (categoryA == 0x80 && categoryB == 0x100) || (categoryA == 0x108 && categoryB == 0x80) || (categoryA == 0x80 && categoryB == 0x108))
    {
        if (!player->isDead) player->healMana(1);
        CCLOG("Player đã va chạm với Monster!");
        // Xác định quái vật nào bị tấn công
        Monster *attackedMonster = nullptr;
        if (categoryA == 0x108 || categoryB == 0x108) {
            boss->takeDamage(player->damage);
        }
        for (auto monster : monsters)
        {
            if (monster->getPhysicsBody() == bodyA || monster->getPhysicsBody() == bodyB)
            {
                attackedMonster = monster;
                break;
            }
        }

        if (attackedMonster)
        {
            attackedMonster->takeDamage(player->damage);
            if (attackedMonster->isDead)
            {
                Vec2 tempPosition = attackedMonster->getPosition();
                tileMap->scheduleOnce([tempPosition, this](float dt)
                                      {
                        Item* currencyItem = Item::spawnItem(tempPosition, ItemType::CURRENCY);
                        tileMap->addChild(currencyItem, 9999); }, 0.05f, "drop_item_from_monster");
            }
        }
        return true;
    }
    if ((categoryA == 0x101 && categoryB == 0x81) || (categoryA == 0x81 && categoryB == 0x101) || (categoryA == 0x100 && categoryB == 0x81) || (categoryA == 0x81 && categoryB == 0x100) || (categoryA == 0x108 && categoryB == 0x81) || (categoryA == 0x81 && categoryB == 0x108))
    {
        CCLOG("Player đã va chạm với Monster!");
        // Xác định quái vật nào bị tấn công
        Monster* attackedMonster = nullptr;
        if (categoryA == 0x108 || categoryB == 0x108) {
            boss->takeDamage(player->damage*2);
        }
        for (auto monster : monsters)
        {
            if (monster->getPhysicsBody() == bodyA || monster->getPhysicsBody() == bodyB)
            {
                attackedMonster = monster;
                break;
            }
        }

        if (attackedMonster)
        {
            attackedMonster->takeDamage(player->damage*2);
            if (attackedMonster->isDead)
            {
                Vec2 tempPosition = attackedMonster->getPosition();
                tileMap->scheduleOnce([tempPosition, this](float dt)
                    {
                        Item* currencyItem = Item::spawnItem(tempPosition, ItemType::CURRENCY);
                        tileMap->addChild(currencyItem, 9999); }, 0.05f, "drop_item_from_monster_by_fire");
            }
        }
        return true;
    }
    if ((categoryA == 0x01 && categoryB == 0x02) || (categoryB == 0x01 && categoryA == 0x02) || (categoryA == 0x80 && categoryB == 0x02) || (categoryB == 0x80 && categoryA == 0x02))
    {
        /*CCLOG("Player đã va chạm với Monster!");
        player->jumpCount = 0;*/
        CCLOG("Player đứng trên object cho phép đứng");
        player->onGroundContact();
        this->isOnGround = true;
        return true;
    }

    if ((categoryA == 0x01 && categoryB == 0x10) || (categoryB == 0x01 && categoryA == 0x10))
    {
        CCLOG("Player đập đá");
        // Áp dụng impulse cho object (nếu object là dynamic)
        float impulseStrength = 500.0f;
        Vec2 impulse = Vec2(-impulseStrength, 0);
        if (categoryB == 0x01)
            bodyA->applyImpulse(impulse);
        else if (categoryA == 0x01)
            bodyB->applyImpulse(impulse);

        player->onGroundContact();
        this->isOnGround = true;

        return true;
    }

    if ((categoryA == 0x08 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x08))
    {
        CCLOG("Player lấy chìa khóa");
        player->setKey(true);
        Node *rockNode = (categoryA == 0x08) ? bodyA->getNode() : bodyB->getNode();
        if (rockNode)
        {
            rockNode->runAction(Sequence::create(FadeOut::create(0.1f), RemoveSelf::create(), nullptr));
        }
        return true;
    }

    if ((categoryA == 0x30 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x30))
    { // Player va vào Chest
        CCLOG("Player đã mở rương! %i", player->getKey());

        if (treasure && player->getKey())
        {
            treasure->openChest("Thinh/chest.png"); // Mở chest (đổi ảnh, spawn vật phẩm bên trong, ...)
            player->setMaxHp(player->getMaxHp() + 3);
            player->heal(3);
        }
        return true;
    }

    if ((categoryA == 0x102 && categoryB == 0x80) || (categoryA == 0x80 && categoryB == 0x102))
    { // Player va vào Chest
        CCLOG("Player đã đập thùng!");
        Chest *openedChest = nullptr;
        for (auto chest : chests)
        {
            if (chest->getPhysicsBody() == bodyA || chest->getPhysicsBody() == bodyB)
            {
                openedChest = chest;
                break;
            }
        }

        if (openedChest)
        {
            openedChest->openChest("Object/Chest/chest_opened.png"); // Mở chest (đổi ảnh, spawn vật phẩm bên trong, ...)
            Vec2 tempPosition = openedChest->getPosition();
            // Nếu cần tạo chest mới sau 1.5 giây
            tileMap->scheduleOnce([tempPosition, this](float dt)
                                  {
            // Spawn item ngay tại vị trí chest
            Item* buffItem = Item::spawnItem(tempPosition, ItemType::BUFF);
            Item* currencyItem = Item::spawnItem(tempPosition, ItemType::CURRENCY);
            tileMap->addChild(buffItem, 9999);
            tileMap->addChild(currencyItem, 9999); }, 0.05f, "spawn_item");
        }
        return true;
    }

    if ((categoryA == 0x103 && categoryB == 0x01) || (categoryA == 0x01 && categoryB == 0x103))
    {
        CCLOG("Player picked up an item!");
        Node *item = (categoryA == 0x103) ? bodyA->getNode() : bodyB->getNode();
        if (item)
        {
            std::string itemName = item->getName();
            if (itemName == "speed_boost")
            {
                CCLOG("Player got speed boost!");
                player->increaseSpeed();
            }
            else if (itemName == "health")
            {
                CCLOG("Player got health!");
                player->heal(1);
            }
            else if (itemName == "gold")
            {
                int goldAmount = cocos2d::RandomHelper::random_int(5, 20); // Random từ 5 đến 20

                CCLOG("Player got %d gold!", goldAmount);
                player->addGold(goldAmount);
                if (player->gold > 40 && player->getMaxHp() < 7) {
                    player->setMaxHp(player->gold / 10);
                }
                auto goldLabel = Label::createWithTTF("+" + std::to_string(goldAmount), "fonts/Marker Felt.ttf", 40);
                goldLabel->setPosition(player->getPosition() + Vec2(0, 50));
                goldLabel->setTextColor(Color4B(255, 215, 0, 255)); // Vàng
                auto scaleUp = ScaleTo::create(0.3f, 1.5f);
                auto fadeOut = FadeOut::create(0.5f);
                auto moveUp = MoveBy::create(0.5f, Vec2(0, 50));
                auto remove = CallFunc::create([goldLabel]()
                                               { goldLabel->removeFromParent(); });
                auto spawn = Spawn::create(scaleUp, moveUp, fadeOut, nullptr);
                auto sequence = Sequence::create(spawn, remove, nullptr);
                goldLabel->runAction(sequence);
                player->getParent()->addChild(goldLabel, 10);
            }

            item->runAction(Sequence::create(FadeOut::create(0.01f), RemoveSelf::create(), nullptr));
        }
        return true;
    }
    if ((categoryA == 0x02 && categoryB == 0x103) || (categoryA == 0x103 && categoryB == 0x02))
    {
        return true;
    }

    return false;
}

void HelloWorld::initKeyboardListener()
{
    if (player->isDead)
        return;
    // Keyboard listener: cập nhật trạng thái phím
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event *event)
    {
        player->onKeyPressed(keyCode);
        switch (keyCode)
        {
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            _isLeftPressed = true;
            break;
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            _isRightPressed = true;
            break;
            // Dùng phím SPACE để nhảy
        case EventKeyboard::KeyCode::KEY_UP_ARROW:
            _isUpPressed = true;
            break;
        case EventKeyboard::KeyCode::KEY_F:
            if (player->getMana() < 4) break;
            player->healMana(-4);
            player->attack2();
            break;
        case EventKeyboard::KeyCode::KEY_SPACE:
            boss->attack1();
            break;
        case EventKeyboard::KeyCode::KEY_G:
            if (player->getMana() < 4) break;
            player->heal(1);
            player->healMana(-4);
            break;
        default:
            break;
        }
    };

    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event)
    {
        player->onKeyReleased(keyCode);
        switch (keyCode)
        {
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            _isLeftPressed = false;
            _isLeftBlocked = false;
            break;
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            _isRightPressed = false;
            _isLeftBlocked = false;
            break;
        case EventKeyboard::KeyCode::KEY_UP_ARROW:
            _isUpPressed = false;
            break;
        default:
            break;
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

void HelloWorld::initGameSchedule(TMXTiledMap *tileMap, Player *player, const Size &visibleSize)
{
    this->schedule([this, tileMap, player, visibleSize](float dt)
                   {
        if (!player->isDead) {
            if (isOnGround)
            {
                //CCLOG("Raycast: isOnGround = true");
                // Nếu di chuyển trái/phải, chỉ cập nhật tileMap nếu không bị block
                /*if (_isLeftPressed && !_isLeftBlocked)
                {
                    Vec2 tilePos = tileMap->getPosition();
                    float panSpeed = 200.0f;
                    tilePos.x += panSpeed * dt;
                    tileMap->setPosition(tilePos);
                }
                if (_isRightPressed && !_isRightBlocked)
                {
                    Vec2 tilePos = tileMap->getPosition();
                    float panSpeed = 200.0f;
                    tilePos.x -= panSpeed * dt;
                    tileMap->setPosition(tilePos);
                }*/

                // Phần xử lý nhảy: nếu nhấn phím SPACE khi đang đứng
                if (_isUpPressed)
                {
                    isOnGround = false;
                    //CCLOG("Raycast: isOnGround = false");
                }

                //// Điều chỉnh vị trí player về giữa màn hình theo chiều X
                //Vec2 desiredPosition = Vec2(visibleSize.width / 2, player->getPositionY());
                //Vec2 currentPos = player->getPosition();
                //float diffX = desiredPosition.x - currentPos.x;
                //float threshold = 5.0f;

                //if (fabs(diffX) > threshold) {
                //    float correctionSpeed = 50.0f;
                //    float correctionX = correctionSpeed * dt * (diffX > 0 ? 1 : -1);
                //    if (fabs(correctionX) > fabs(diffX)) correctionX = diffX;
                //    player->setPositionX(currentPos.x + correctionX);
                //}
                //if (fabs(diffX) > threshold) {
                //    float correctionSpeed = 50.0f;
                //    float correctionX = correctionSpeed * dt * (diffX > 0 ? 1 : -1);
                //    if (fabs(correctionX) > fabs(diffX)) correctionX = diffX;
                //    player->setPositionX(currentPos.x + correctionX);
                //}

            }
            else
            {
                //CCLOG("Raycast: isOnGround = false");
                /*if (_isLeftPressed && !_isLeftBlocked)
                {
                    Vec2 tilePos = tileMap->getPosition();
                    float panSpeed = 200.0f;
                    tilePos.x += panSpeed * dt;
                    tileMap->setPosition(tilePos);
                }
                if (_isRightPressed && !_isRightBlocked)
                {
                    Vec2 tilePos = tileMap->getPosition();
                    float panSpeed = 200.0f;
                    tilePos.x -= panSpeed * dt;
                    tileMap->setPosition(tilePos);
                }*/
            }
        } }, "update_game");
}

void HelloWorld::initMap2(TMXTiledMap *tileMap)
{
    // Tạo monster
    for (int i = 0; i < 5; i++)
    {
        Vec2 spawnPos = Vec2(1200 + i * 1000, 1500);
        Monster *newMonster = FlyingMonster::spawnMonster(spawnPos);
        tileMap->addChild(newMonster, 9999);
        monsters.push_back(newMonster);
    }
    for (int i = 0; i < 5; i++)
    {
        Vec2 spawnPos = Vec2(1000 + i * 1000, 500);
        Monster *newMonster = FlyingMonster::spawnMonster(spawnPos);
        tileMap->addChild(newMonster, 9999);
        monsters.push_back(newMonster);
    }
    for (int i = 0; i < 5; i++)
    {
        Vec2 spawnPos = Vec2(1000 + i * 1000, 30);
        Monster *newMonster = GroundMonster::spawnMonster(spawnPos);
        tileMap->addChild(newMonster, 9999);
        monsters.push_back(newMonster);
    }
    std::vector<Vec2> spawnPositions = {
        Vec2(5755, 193),
        Vec2(4600, 1280)};

    for (const auto &pos : spawnPositions)
    {
        Chest *chest = Chest::createChest(pos);
        tileMap->addChild(chest, 9999);
        chests.push_back(chest);
    }
}

bool HelloWorld::initMap3(TMXTiledMap *tileMap)
{
    auto rock1 = Sprite::create("Thinh/rock.png"); // Tạo object từ hình ảnh
    if (rock1 == nullptr)
    {
        problemLoading("'Thinh/rock.png'");
        return false;
    }
    rock1->setPosition(Vec2(10000, 2800)); // Đặt vị trí ban đầu

    // Tạo PhysicsBody với kích thước dựa trên object
    auto body = PhysicsBody::createBox(rock1->getContentSize(), PhysicsMaterial(1.0f, 0.5f, 0.3f));
    body->setDynamic(true);       // Cho phép di chuyển
    body->setGravityEnable(true); // Nếu không muốn bị ảnh hưởng bởi trọng lực
    body->setCategoryBitmask(0x10);
    body->setCollisionBitmask(0x01 | 0x02 | 0x04); // Cài đặt va chạm
    body->setContactTestBitmask(0x01);

    rock1->setPhysicsBody(body); // Gán PhysicsBody vào object

    tileMap->addChild(rock1, 12345); // Thêm vào scene

    auto rock2 = Sprite::create("Thinh/rock.png"); // Tạo object từ hình ảnh
    if (rock2 == nullptr)
    {
        problemLoading("'Thinh/rock.png'");
        return false;
    }
    rock2->setPosition(Vec2(11300, 2800)); // Đặt vị trí ban đầu

    // Tạo PhysicsBody với kích thước dựa trên object
    body = PhysicsBody::createBox(rock2->getContentSize(), PhysicsMaterial(1.0f, 0.5f, 0.3f));
    body->setDynamic(true);       // Cho phép di chuyển
    body->setGravityEnable(true); // Nếu không muốn bị ảnh hưởng bởi trọng lực
    body->setCategoryBitmask(0x10);
    body->setCollisionBitmask(0x01 | 0x02 | 0x04); // Cài đặt va chạm
    body->setContactTestBitmask(0x01);

    rock2->setPhysicsBody(body); // Gán PhysicsBody vào object

    tileMap->addChild(rock2, 12345); // Thêm vào scene

    auto key = Sprite::create("Thinh/key.png"); // Tạo object từ hình ảnh
    if (key == nullptr)
    {
        problemLoading("'Thinh/key.png'");
        return false;
    }
    key->setPosition(Vec2(6100, 2800)); // Đặt vị trí ban đầu

    // Tạo PhysicsBody với kích thước dựa trên object
    body = PhysicsBody::createBox(key->getContentSize(), PhysicsMaterial(1.0f, 0.5f, 0.3f));
    body->setDynamic(true);       // Cho phép di chuyển
    body->setGravityEnable(true); // Nếu không muốn bị ảnh hưởng bởi trọng lực
    body->setCategoryBitmask(0x08);
    body->setCollisionBitmask(0x01 | 0x02); // Cài đặt va chạm
    body->setContactTestBitmask(0x01);

    key->setPhysicsBody(body); // Gán PhysicsBody vào object

    tileMap->addChild(key, 12345); // Thêm vào scene

    treasure = Chest::createSpecialChest(Vec2(11700, 1800));
    if (treasure == nullptr)
    {
        problemLoading("'Thinh/treasure.png'");
        return false;
    }
    tileMap->addChild(treasure, 12345);
}


void HelloWorld::initMap4(TMXTiledMap* tileMap) {
    auto left = Vec2(6300,1000);
    auto right = Vec2(7800,1000);
    boss = Boss::create(left, right);
    tileMap->addChild(boss, 123);
    tileMap->addChild(boss->right,123);
    tileMap->addChild(boss->left,123);
}



void HelloWorld::randomAttack(float dt) {
    if (boss->getHealth()<=0) return;  // Kiểm tra boss có tồn tại không

    boss->attack1();
    CCLOG("Boss");
    // Tiếp tục đặt lịch tấn công tiếp theo với thời gian ngẫu nhiên
    scheduleNextAttack();
}

void HelloWorld::scheduleNextAttack() {
    float delay = 2.0f + (rand() % 3000) / 1000.0f;  // Ngẫu nhiên từ 1.0s đến 4.0s
    scheduleOnce(CC_SCHEDULE_SELECTOR(HelloWorld::randomAttack), delay);

}

void HelloWorld::update(float delta)
{
    if (!player->isDead)
    {
        Vec2 playerPosInTileMap = tileMap->convertToNodeSpace(player->getPosition());
        if (!BossAttack && playerPosInTileMap.x > 12500 && playerPosInTileMap.y < 2000) {
            BossAttack = true;
            scheduleNextAttack();
            // Giả sử bossHealthBar được khai báo là biến thành viên của lớp HelloWorld
            if (!bossHealthBar) {
                auto healthSprite = Sprite::create("Thinh/bossHealthBar.png");
                bossHealthBar = ProgressTimer::create(healthSprite);
                bossHealthBar->setType(ProgressTimer::Type::BAR);
                // Thanh điền từ trái sang phải
                bossHealthBar->setBarChangeRate(Vec2(1, 0));
                bossHealthBar->setMidpoint(Vec2(0, 0.5));

                // Đặt vị trí ở đầu màn hình (bạn có thể thay đổi vị trí theo ý muốn)
                auto visibleSize = Director::getInstance()->getVisibleSize();
                bossHealthBar->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - healthSprite->getContentSize().height / 2 - 10));

                // Thêm vào scene với thứ tự z phù hợp
                uiLayer->addChild(bossHealthBar, 9999);

                // Khởi tạo phần trăm ban đầu là 100%
                bossHealthBar->setPercentage(100);
            }

        }
        player->update(delta);
        boss->update(delta);
        live->update(delta, player->getHp());
        int index = static_cast<int>((static_cast<float>(player->getMana()) / player->getMaxMana()) * 17);
        board->update(delta, index);
        /*CCLOG("Player đã mở rương! %i", player->getKey());*/
       /* CCLOG("Mạng Player: %d", player->getMaxHp());*/
        uiLayer->setPosition(this->getPosition() * -1);
        std::string goldText = "Gold: " + std::to_string(player->gold);
        goldLabel->setString(goldText);
        if (bossHealthBar && boss->getHealth() > 0) {
            float percentage = (static_cast<float>(boss->getHealth()) / boss->getMaxheath()) * 100.0f;
            bossHealthBar->setPercentage(percentage);
        }
        if (boss->getHealth() <= 0 && bossHealthBar) {
            bossHealthBar->setVisible(false);
            // hoặc: this->removeChild(bossHealthBar);
        }
        if (bossHealthBar) {
            CCLOG("Cập nhật thanh máu: %f", bossHealthBar->getPercentage());
        }
        else {
            CCLOG("bossHealthBar đã bị mất!");
        }

        /*CCLOG("Vị trí nhân vật trên tileMap: x = %f, y = %f", playerPosInTileMap.x, playerPosInTileMap.y);
        CCLOG("BossAttack = %d", BossAttack);*/
    }
    else
    {
        AudioEngine::stopAll();
        auto menuScene = MenuScene::create();
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, menuScene));
    }
}

void HelloWorld::menuCloseCallback(Ref *pSender)
{
    Director::getInstance()->end();
}
