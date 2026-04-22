#ifndef WATERCURTAINWIDGET_H
#define WATERCURTAINWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QImage>

class WaterCurtainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaterCurtainWidget(QWidget* parent = nullptr);
    ~WaterCurtainWidget();

    // 加载BMP图片作为水帘形状
    bool loadCurtainTexture(const QString& filepath);
    void setParticleCount(int count);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateParticles();

private:
    struct Particle {
        float x, y, z;
        float vx, vy, vz;
        float life;
    };

    void initParticles();
    bool shouldSpawnAt(float x, float z) const;  // 根据BMP判断是否生成粒子

    QVector<Particle> m_particles;
    QTimer m_updateTimer;
    int m_particleCount;
    float m_lastTime;

    // BMP纹理数据
    QImage m_curtainImage;
    int m_curtainWidth;
    int m_curtainHeight;
};

#endif // WATERCURTAINWIDGET_H