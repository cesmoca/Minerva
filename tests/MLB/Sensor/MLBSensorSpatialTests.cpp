#include <Factories/MAOFactory.h>
#include <MLB/Sensor/MLBSensorCollision.h>
#include <MLB/Sensor/MLBSensorNear.h>
#include <gtest/gtest.h>

namespace {

class TestPositionator : public MAOPositionator3D {
public:
    using MAOPositionator3D::MAOPositionator3D;

    cv::Mat& getPosMatrix() override { return _posMatrix; }

    void setTranslation(float x, float y, float z) {
        _posMatrix.at<float>(3, 0) = x;
        _posMatrix.at<float>(3, 1) = y;
        _posMatrix.at<float>(3, 2) = z;
    }
};

class TestRenderable : public MAORenderable3D {
public:
    explicit TestRenderable(const std::string& name)
        : MAORenderable3D(name, 1.0F) {}

    ~TestRenderable() override { delete _collisionShape; }

    cv::Mat& getPosMatrix() override { return _posMatrix; }

    void setTranslation(float x, float y, float z) {
        _posMatrix.at<float>(3, 0) = x;
        _posMatrix.at<float>(3, 1) = y;
        _posMatrix.at<float>(3, 2) = z;
    }

    void generateCollisionShape(int type) override {
        if (type == MAO_BOX_SHAPE) {
            setBoxShape(new btBoxShape(btVector3(1.0F, 1.0F, 1.0F)));
        }
    }

private:
    void _drawMAO() override {}
    void _drawMAONoTexture() override {}
};

TEST(MLBSensorNearTest, FindsPositionedObjectsWithTheRequestedProperty) {
    TestPositionator parent("near-parent");
    TestPositionator candidate("near-candidate");
    parent.setPositioned(true);
    candidate.setPositioned(true);
    candidate.setTranslation(2.0F, 0.0F, 0.0F);
    candidate.addPropertyBoolean("target", true);
    MAOFactory::getInstance()->getVectorMAOPositionator3D().push_back(&candidate);

    MLBSensorNear sensor("near", parent, "target", 3.0F);
    sensor.evaluate();

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORNEAR);
    EXPECT_TRUE(sensor.getState());
    EXPECT_FLOAT_EQ(sensor.mPyGetMinDistance(), 3.0F);
    EXPECT_EQ(sensor.mPyGetNearProperty(), "target");

    sensor.mPySetMinDistance(1.0F);
    sensor.mPySetNearProperty("other");
    EXPECT_FLOAT_EQ(sensor.mPyGetMinDistance(), 1.0F);
    EXPECT_EQ(sensor.mPyGetNearProperty(), "other");
}

TEST(MLBSensorCollisionTest, UsesBulletShapesAndPropertyFiltering) {
    TestRenderable parent("collision-parent");
    TestRenderable candidate("collision-candidate");
    parent.setPositioned(true);
    candidate.setPositioned(true);
    parent.generateCollisionShape(MAO_BOX_SHAPE);
    candidate.generateCollisionShape(MAO_BOX_SHAPE);
    candidate.addPropertyBoolean("solid", true);
    MAOFactory::getInstance()->getVectorMAORenderable3D().push_back(&candidate);

    MLBSensorCollision sensor("collision", parent, "solid");
    sensor.evaluate();

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORCOLLISION);
    EXPECT_TRUE(sensor.getState());
    EXPECT_EQ(sensor.mPyGetCollisionProperty(), "solid");

    sensor.mPySetCollisionProperty("missing");
    sensor.evaluate();
    EXPECT_FALSE(sensor.getState());
}

}  // namespace
