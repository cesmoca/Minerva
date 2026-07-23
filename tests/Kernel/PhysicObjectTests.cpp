#include <Kernel/PhysicDynamicObject.h>
#include <Kernel/PhysicObject.h>
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
            setBoxShape(new btBoxShape(btVector3(1.0F, 2.0F, 3.0F)));
        }
    }

private:
    void _drawMAO() override {}
    void _drawMAONoTexture() override {}
};

TEST(PhysicObjectTest, BuildsRigidBodyFromRenderableState) {
    TestPositionator reference("reference");
    TestRenderable3D renderable("renderable", 1.0F);
    renderable.setGlobalReference(reference);
    renderable.setMass(4.0F);
    renderable.generateCollisionShape(MAO_BOX_SHAPE);

    PhysicObject object(&renderable);

    EXPECT_EQ(object.getMAO(), &renderable);
    ASSERT_NE(object.getRigidBody(), nullptr);
    EXPECT_FLOAT_EQ(object.getRigidBody()->getInvMass(), 0.25F);
    EXPECT_NE(object.getRigidBody()->getMotionState(), nullptr);
}

TEST(PhysicObjectTest, ConfiguresStaticBodyAsKinematic) {
    TestPositionator reference("reference");
    TestRenderable3D renderable("renderable", 1.0F);
    renderable.setGlobalReference(reference);
    renderable.setMass(0.0F);
    renderable.generateCollisionShape(MAO_BOX_SHAPE);

    PhysicObject object(&renderable);

    EXPECT_NE(
        object.getRigidBody()->getCollisionFlags()
            & btCollisionObject::CF_KINEMATIC_OBJECT,
        0);
    EXPECT_EQ(
        object.getRigidBody()->getActivationState(),
        DISABLE_DEACTIVATION);
}

TEST(PhysicDynamicObjectTest, StoresCreationStateOffsetAndImpulse) {
    TestPositionator reference("reference");
    TestRenderable3D renderable("renderable", 1.0F);
    renderable.setGlobalReference(reference);
    renderable.setMass(2.0F);
    renderable.generateCollisionShape(MAO_BOX_SHAPE);

    cv::Mat offset = cv::Mat::eye(4, 4, CV_32F);
    offset.at<float>(3, 0) = 4.0F;
    btVector3 impulse(1.0F, 2.0F, 3.0F);
    PhysicDynamicObject object(&renderable, reference, &offset, &impulse);

    EXPECT_FALSE(object.isCreated());
    EXPECT_EQ(&object.getCreationRef(), &reference);
    EXPECT_FLOAT_EQ(object.getOffset().at<float>(3, 0), 4.0F);
    EXPECT_EQ(object.getImpulse(), impulse);

    object.setCreated(true);
    EXPECT_TRUE(object.isCreated());
}

TEST(PhysicDynamicObjectTest, DefaultsToIdentityOffsetAndZeroImpulse) {
    TestPositionator reference("reference");
    TestRenderable3D renderable("renderable", 1.0F);
    renderable.setGlobalReference(reference);
    renderable.setMass(1.0F);
    renderable.generateCollisionShape(MAO_BOX_SHAPE);

    PhysicDynamicObject object(&renderable, reference);

    EXPECT_FLOAT_EQ(object.getOffset().at<float>(0, 0), 1.0F);
    EXPECT_FLOAT_EQ(object.getOffset().at<float>(3, 3), 1.0F);
    EXPECT_EQ(object.getImpulse(), btVector3(0.0F, 0.0F, 0.0F));
}

}  // namespace
