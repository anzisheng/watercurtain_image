#include "WaterCurtainWidget.h"
#include <QPainter>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

WaterCurtainWidget::WaterCurtainWidget(QWidget* parent)
    : QWidget(parent)
    , m_particleCount(8000)
    , m_lastTime(0.0f)
    , m_curtainWidth(0)
    , m_curtainHeight(0)
{
    setMinimumSize(800, 600);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_updateTimer.setInterval(16);
    connect(&m_updateTimer, &QTimer::timeout, this, &WaterCurtainWidget::updateParticles);
    m_updateTimer.start();
}

WaterCurtainWidget::~WaterCurtainWidget()
{
}

bool WaterCurtainWidget::loadCurtainTexture(const QString& filepath)
{
    if (m_curtainImage.load(filepath)) {
        m_curtainImage = m_curtainImage.convertToFormat(QImage::Format_Grayscale8);
        m_curtainWidth = m_curtainImage.width();
        m_curtainHeight = m_curtainImage.height();
        qDebug() << "Loaded BMP:" << m_curtainWidth << "x" << m_curtainHeight;
        initParticles();
        return true;
    }
    qDebug() << "No BMP loaded, using default pattern";
    return false;
}

void WaterCurtainWidget::setParticleCount(int count)
{
    m_particleCount = count;
    initParticles();
}

bool WaterCurtainWidget::shouldSpawnAt(float x, float z) const
{
    if (m_curtainImage.isNull()) {
        // 默认显示心形
        float heartX = x / 2.5f;
        float heartZ = z / 2.0f;
        float heartFormula = pow(heartX * heartX + heartZ * heartZ - 1, 3) - heartX * heartX * heartZ * heartZ * heartZ;
        return heartFormula < 0;
    }

    float u = (x + 5.0f) / 10.0f;
    float v = (z + 4.0f) / 8.0f;

    if (u < 0 || u > 1 || v < 0 || v > 1) return false;

    int ix = qBound(0, static_cast<int>(u * m_curtainWidth), m_curtainWidth - 1);
    int iy = qBound(0, static_cast<int>(v * m_curtainHeight), m_curtainHeight - 1);

    int gray = qGray(m_curtainImage.pixel(ix, iy));
    float density = gray / 255.0f;

    return density > 0.3f;  // 阈值0.3，只显示较亮区域
}

void WaterCurtainWidget::initParticles()
{
    m_particles.clear();

    // 方法：遍历整个X-Z网格，根据BMP密度决定是否放置粒子
    float xStart = -5.0f;
    float xEnd = 5.0f;
    float zStart = -4.0f;
    float zEnd = 4.0f;

    // 降低采样密度，让粒子之间有间隙，形状更清晰
    int gridX = 80;
    int gridZ = 64;

    float stepX = (xEnd - xStart) / gridX;
    float stepZ = (zEnd - zStart) / gridZ;

    for (int ix = 0; ix < gridX && m_particles.size() < m_particleCount; ++ix) {
        for (int iz = 0; iz < gridZ && m_particles.size() < m_particleCount; ++iz) {
            float x = xStart + ix * stepX + stepX / 2;
            float z = zStart + iz * stepZ + stepZ / 2;

            if (shouldSpawnAt(x, z)) {
                Particle p;
                p.x = x;
                p.z = z;
                p.y = 4.5f - QRandomGenerator::global()->generateDouble() * 6.0f;  // 随机Y位置
                p.vy = -1.0f - QRandomGenerator::global()->generateDouble() * 2.0f;
                p.life = 1.0f;
                m_particles.append(p);
            }
        }
    }

    m_particleCount = m_particles.size();
    qDebug() << "Initialized" << m_particleCount << "particles";

    // 打印一些调试信息
    if (!m_curtainImage.isNull()) {
        qDebug() << "BMP width:" << m_curtainWidth << "height:" << m_curtainHeight;
        // 采样BMP中心区域
        int cx = m_curtainWidth / 2;
        int cy = m_curtainHeight / 2;
        qDebug() << "Center pixel gray:" << qGray(m_curtainImage.pixel(cx, cy));
    }
}

void WaterCurtainWidget::updateParticles()
{
    float now = QElapsedTimer().nsecsElapsed() / 1e9f;
    float deltaTime = now - m_lastTime;
    m_lastTime = now;

    if (deltaTime > 0.033f) deltaTime = 0.033f;
    if (deltaTime < 0.001f) deltaTime = 0.016f;

    float gravity = 7.0f;

    for (int i = 0; i < m_particles.size(); ++i) {
        Particle& p = m_particles[i];

        p.vy += gravity * deltaTime;
        p.y += p.vy * deltaTime;

        // 超出底部则重置到顶部（保持X,Z不变）
        if (p.y < -3.8f) {
            p.y = 4.5f;
            p.vy = -1.0f - QRandomGenerator::global()->generateDouble() * 2.0f;
        }

        if (p.y > 5.0f) {
            p.y = 4.5f;
            p.vy = -1.0f - QRandomGenerator::global()->generateDouble() * 2.0f;
        }
    }

    update();
}

void WaterCurtainWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 黑色背景，对比更明显
    painter.fillRect(rect(), QColor(0, 0, 0));

    int w = width();
    int h = height();

    // 正交投影，直接显示X-Z平面，更容易看清形状
    // 相机正对着水帘平面
    float scaleX = w / 10.0f;   // X范围 -5 到 5
    float scaleY = h / 8.0f;    // Z范围 -4 到 4（实际是Z，但显示在屏幕Y）

    // 先绘制半透明背景网格（可选，帮助定位）
    painter.setPen(QColor(50, 50, 80));
    for (int i = -5; i <= 5; i++) {
        int screenX = (i + 5) * scaleX;
        painter.drawLine(screenX, 0, screenX, h);
    }
    for (int i = -4; i <= 4; i++) {
        int screenY = (4 - i) * scaleY;
        painter.drawLine(0, screenY, w, screenY);
    }

    // 统计每个(X,Z)位置的粒子数量（用于颜色强度）
    QHash<QString, int> densityMap;
    for (const auto& p : m_particles) {
        QString key = QString("%1,%2").arg(p.x, 0, 'f', 1).arg(p.z, 0, 'f', 1);
        densityMap[key] = densityMap.value(key, 0) + 1;
    }

    // 绘制粒子 - 使用不同颜色表示密度
    for (const auto& p : m_particles) {
        // 屏幕坐标（正交投影，直接映射）
        float screenX = (p.x + 5.0f) * scaleX;
        float screenY = (4.0f - p.z) * scaleY;  // Z轴映射到屏幕Y

        if (screenX < 0 || screenX > w || screenY < 0 || screenY > h) continue;

        // 获取BMP密度
        float density = 0.8f;
        if (!m_curtainImage.isNull()) {
            float u = (p.x + 5.0f) / 10.0f;
            float v = (p.z + 4.0f) / 8.0f;
            if (u >= 0 && u <= 1 && v >= 0 && v <= 1) {
                int ix = qBound(0, static_cast<int>(u * m_curtainWidth), m_curtainWidth - 1);
                int iy = qBound(0, static_cast<int>(v * m_curtainHeight), m_curtainHeight - 1);
                density = qGray(m_curtainImage.pixel(ix, iy)) / 255.0f;
            }
        }

        // 根据密度设置颜色：白色（高密度）到蓝色（低密度）
        int brightness = 100 + (int)(density * 155);

        // 大小固定，更容易看清形状
        float size = 4.0f;

        // 速度影响透明度
        float alpha = 0.6f + (qAbs(p.vy) / 15.0f) * 0.4f;

        painter.setBrush(QColor(brightness, brightness, 255, (int)(alpha * 255)));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(screenX, screenY), size, size);
    }

    // 显示信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 30, "Water Curtain - BMP Shape Display");
    painter.drawText(10, 55, QString("Particles: %1").arg(m_particles.size()));

    if (!m_curtainImage.isNull()) {
        painter.drawText(10, 80, QString("BMP: %1x%2").arg(m_curtainWidth).arg(m_curtainHeight));
        painter.drawText(10, 105, "White areas = high density, Blue areas = low density");
        painter.drawText(10, 130, "Particles fall vertically, shape preserved!");
    }
    else {
        painter.drawText(10, 80, "No BMP loaded - Showing heart shape");
        painter.drawText(10, 105, "Place 'water_curtain.bmp' in program folder");
        painter.drawText(10, 130, "BMP should be grayscale, white = particles, black = no particles");
    }
}