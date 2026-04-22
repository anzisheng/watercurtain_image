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
    void startFalling();  // 开始下落

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;  // 按空格开始下落

private slots:
    void updateParticles();

private:
    struct Particle {
        float x, z;      // 固定位置
        float y;         // 当前Y坐标
        float vy;        // Y方向速度
        bool active;
    };

    void initParticles();
    bool shouldSpawnAt(float x, float z) const;

    QVector<Particle> m_particles;
    QTimer m_updateTimer;
    int m_particleCount;
    float m_lastTime;
    bool m_isFalling;    // 是否开始下落

    QImage m_curtainImage;
    int m_curtainWidth;
    int m_curtainHeight;
};

#endif // WATERCURTAINWIDGET_H