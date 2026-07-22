#include <MAO/MAORenderable3D.h>
#include <MAO/MAORenderable3DLine.h>
#include <MAO/MAORenderable3DModel.h>
#include <MAO/MAORenderable3DPath.h>
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

TEST(MAORenderable3DTest, ManagesPoseVisibilityAndCollisionState) {
    TestPositionator reference("reference");
    reference.setPositioned(true);
    TestRenderable3D renderable("renderable", 2.5F);

    EXPECT_EQ(renderable.getType(), T_MAORENDERABLE3D);
    EXPECT_FLOAT_EQ(renderable.getSize(), 2.5F);
    EXPECT_FALSE(renderable.isVisible());
    EXPECT_FALSE(renderable.isPhysic());
    EXPECT_TRUE(renderable.isActive());
    EXPECT_EQ(renderable.getCollisionShape(), nullptr);

    renderable.setGlobalReference(reference);
    renderable.setMass(4.0F);
    renderable.setPhysic(true);
    renderable.setCollisionShapeType(MAO_BOX_SHAPE);

    EXPECT_EQ(renderable.getGlobalReference(), &reference);
    EXPECT_TRUE(renderable.isVisible());
    EXPECT_FLOAT_EQ(renderable.getMass(), 4.0F);
    EXPECT_TRUE(renderable.isPhysic());
    ASSERT_NE(renderable.getCollisionShape(), nullptr);
    EXPECT_EQ(renderable.getCollisionShapeType(), MAO_BOX_SHAPE);
}

TEST(MAORenderable3DLineTest, StoresPresentationPropertiesAndPoints) {
    MAORenderable3DLine line("line", 3.0F, 10, 20, 30);

    EXPECT_EQ(line.getType(), T_MAORENDERABLE3DLINE);
    EXPECT_FLOAT_EQ(line.getSize(), 3.0F);
    EXPECT_EQ(line.getR(), 10);
    EXPECT_EQ(line.getG(), 20);
    EXPECT_EQ(line.getB(), 30);
    EXPECT_NO_THROW(line.setPoints(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F));
}

TEST(MAORenderable3DModelTest, StoresModelIdentityAndScale) {
    MAORenderable3DModel model(
        "model", boost::filesystem::path("model.orej"), 1.5F);

    EXPECT_EQ(model.getType(), T_MAORENDERABLE3DMODEL);
    EXPECT_FLOAT_EQ(model.getSize(), 1.5F);
    EXPECT_EQ(model.getCollisionShape(), nullptr);
}

TEST(MAORenderable3DPathTest, StoresPathPresentationAndAcceptsPoints) {
    MAORenderable3DPath path("path", 2.0F, 40, 50, 60);

    EXPECT_EQ(path.getType(), T_MAORENDERABLE3DPATH);
    EXPECT_EQ(path.getR(), 40);
    EXPECT_EQ(path.getG(), 50);
    EXPECT_EQ(path.getB(), 60);
    EXPECT_FALSE(path.getVisiblePoint());
    EXPECT_NO_THROW(path.addPoint());
    EXPECT_NO_THROW(path.removePoints());
}

}  // namespace
