#include <Controllers/PhysicsController.h>
#include <MAO/MAORenderable3D.h>
#include <gtest/gtest.h>

namespace {

class TestPositionator : public MAOPositionator3D {
public:
    using MAOPositionator3D::MAOPositionator3D;

    cv::Mat& getPosMatrix() override { return _posMatrix; }
};

class TestRenderable3D : public MAORenderable3D {
public:
    using MAORenderable3D::MAORenderable3D;

    ~TestRenderable3D() override { delete _collisionShape; }

    void generateCollisionShape(int type) override {
        if (type == MAO_BOX_SHAPE) {
            setBoxShape(new btBoxShape(btVector3(1.0F, 1.0F, 1.0F)));
        }
    }

private:
    void _drawMAO() override {}
    void _drawMAONoTexture() override {}
};

TEST(PhysicsControllerTest, InitializesWorldAndConfiguresGround) {
    PhysicsController controller;
    TestPositionator ground("ground");
    ground.setPositioned(true);
    std::string axis = "Z";

    EXPECT_FALSE(controller.isActive());
    EXPECT_ANY_THROW(controller.getMAOGround());

    controller.initPhysics();
    controller.setMAOGround(ground, axis, -9.8F, false);

    EXPECT_TRUE(controller.isActive());
    EXPECT_EQ(&controller.getMAOGround(), &ground);
    EXPECT_FALSE(controller.shadowsActive());
    ASSERT_NE(controller._world, nullptr);
    EXPECT_FLOAT_EQ(controller._world->getGravity().z(), -9.8F);
    EXPECT_ANY_THROW(controller.initPhysics());
}

TEST(PhysicsControllerTest, AddsAndRemovesStaticAndDynamicBodies) {
    PhysicsController controller;
    TestPositionator ground("ground");
    ground.setPositioned(true);
    std::string axis = "Y";
    controller.initPhysics();
    controller.setMAOGround(ground, axis, -9.8F, false);

    TestPositionator reference("reference");
    reference.setPositioned(true);

    TestRenderable3D staticBody("static", 1.0F);
    staticBody.setGlobalReference(reference);
    controller.addStaticRigidBody(staticBody);

    EXPECT_TRUE(staticBody.isPhysic());
    EXPECT_FLOAT_EQ(staticBody.getMass(), 0.0F);
    EXPECT_TRUE(controller.removeStaticRigidBody(&staticBody));

    TestRenderable3D dynamicBody("dynamic", 1.0F);
    dynamicBody.setGlobalReference(reference);
    controller.addDynamicRigidBody(dynamicBody, reference, 2.0F);

    EXPECT_TRUE(dynamicBody.isPhysic());
    EXPECT_FALSE(dynamicBody.isActive());
    EXPECT_FLOAT_EQ(dynamicBody.getMass(), 2.0F);
    EXPECT_TRUE(controller.removeDynamicRigidBody(&dynamicBody));
}

TEST(PhysicsControllerTest, DetectsOverlappingAndSeparatedBoxes) {
    PhysicsController controller;
    TestPositionator firstReference("first-reference");
    TestPositionator secondReference("second-reference");
    firstReference.setPositioned(true);
    secondReference.setPositioned(true);

    TestRenderable3D first("first", 1.0F);
    TestRenderable3D second("second", 1.0F);
    first.setGlobalReference(firstReference);
    second.setGlobalReference(secondReference);
    first.generateCollisionShape(MAO_BOX_SHAPE);
    second.generateCollisionShape(MAO_BOX_SHAPE);

    EXPECT_TRUE(controller.collision(&first, &second));

    secondReference.getPosMatrix().at<float>(3, 0) = 10.0F;

    EXPECT_FALSE(controller.collision(&first, &second));
}

}  // namespace
