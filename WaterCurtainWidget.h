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

    bool loadCurtainTexture(const QString& filepath);
    void setParticleCount(int count);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateParticles();

private:
    struct Particle {
        float x, z;      // 固定位置（根据BMP决定）
        float y;         // 当前Y坐标
        float vy;        // Y方向速度
        float life;
    };

    void initParticles();
    bool shouldSpawnAt(float x, float z) const;

    QVector<Particle> m_particles;
    QTimer m_updateTimer;
    int m_particleCount;
    float m_lastTime;

    QImage m_curtainImage;
    int m_curtainWidth;
    int m_curtainHeight;
};

#endif // WATERCURTAINWIDGET_H