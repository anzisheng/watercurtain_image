#include "WaterCurtainWidget.h"
#include <QPainter>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>
#include <QKeyEvent>

WaterCurtainWidget::WaterCurtainWidget(QWidget* parent)
    : QWidget(parent)
    , m_particleCount(8000)
    , m_lastTime(0.0f)
    , m_isFalling(false)
    , m_curtainWidth(0)
    , m_curtainHeight(0)
{
    setMinimumSize(800, 600);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);

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
void WaterCurtainWidget::startFalling()
{
    if (m_isFalling) {
        m_isFalling = false;
        qDebug() << "Paused";
    }
    else {
        // 重置所有粒子：活跃、位置到顶部、初速度
        for (int i = 0; i < m_particles.size(); ++i) {
            m_particles[i].active = true;
            m_particles[i].y = 4.0f;
            m_particles[i].vy = -5.0f - QRandomGenerator::global()->generateDouble() * 7.5f;
        }
        m_isFalling = true;
        m_lastTime = QElapsedTimer().nsecsElapsed() / 1e9f;
        qDebug() << "Start falling! All particles reset";
    }
}

void WaterCurtainWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space) {
        startFalling();
    }
}

bool WaterCurtainWidget::shouldSpawnAt(float x, float z) const
{
    if (m_curtainImage.isNull()) {
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

    return density > 0.3f;
}

void WaterCurtainWidget::initParticles()
{
    m_particles.clear();

    float xStart = -5.0f;
    float xEnd = 5.0f;
    float zStart = -4.0f;
    float zEnd = 4.0f;

    int gridX = 100;
    int gridZ = 80;

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
                p.y = 0.0f;
                p.vy = 0.0f;
                p.active = true;
                m_particles.append(p);
            }
        }
    }

    m_particleCount = m_particles.size();
    qDebug() << "Initialized" << m_particleCount << "particles - Press SPACE to start/stop";
}

void WaterCurtainWidget::updateParticles()
{
    if (!m_isFalling) {
        return;
    }

    float now = QElapsedTimer().nsecsElapsed() / 1e9f;
    float deltaTime = now - m_lastTime;
    m_lastTime = now;

    if (deltaTime > 0.033f) deltaTime = 0.033f;
    if (deltaTime < 0.001f) deltaTime = 0.016f;

    float gravity = 40.0f;

    for (int i = 0; i < m_particles.size(); ++i) {
        Particle& p = m_particles[i];

        if (!p.active) continue;

        p.vy += gravity * deltaTime;
        p.y += p.vy * deltaTime;

        // 超出底部则标记为不可见（消失）
        if (p.y < -5.0f) {
            p.active = false;
        }
    }

    update();
}
void WaterCurtainWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(0, 0, 0));

    int w = width();
    int h = height();

    float marginX = w * 0.2f;
    float marginY = h * 0.2f;
    float drawW = w - 2 * marginX;
    float drawH = h - 2 * marginY;

    float scaleX = drawW / 10.0f;
    float scaleZ = drawH / 8.0f;

    // 绘制所有活跃粒子
    for (const auto& p : m_particles) {
        if (!p.active) continue;  // 只绘制活跃粒子

        float screenX = marginX + (p.x + 5.0f) * scaleX;

        float screenY;
        if (m_isFalling) {
            float baseY = marginY + (4.0f - p.z) * scaleZ;
            float yOffset = (p.y) * (scaleZ * 0.1f);
            screenY = baseY + yOffset;
        }
        else {
            screenY = marginY + (4.0f - p.z) * scaleZ;
        }

        if (screenX < -20 || screenX > w + 20 || screenY < -20 || screenY > h + 20) continue;

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

        float size = 2.5f + density * 5.0f;

        int r = 80 + (int)(density * 175);
        int g = 120 + (int)(density * 135);
        int b = 200 + (int)(density * 55);

        int alpha = 200;
        if (m_isFalling) {
            float heightFactor = (p.y + 3.5f) / 7.5f;
            heightFactor = qBound(0.2f, heightFactor, 1.0f);
            alpha = 150 + (int)(heightFactor * 105);
        }

        painter.setBrush(QColor(r, g, b, alpha));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(screenX, screenY), size, size);
    }

    // 统计剩余活跃粒子数
    int activeCount = 0;
    for (const auto& p : m_particles) {
        if (p.active) activeCount++;
    }

    // 显示信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 11));
    painter.drawText(10, 25, "Water Curtain - BMP Shape Display");
    painter.drawText(10, 45, QString("Total Particles: %1").arg(m_particles.size()));
    painter.drawText(10, 65, QString("Active Particles: %1").arg(activeCount));

    if (!m_curtainImage.isNull()) {
        painter.drawText(10, 85, QString("BMP: %1x%2").arg(m_curtainWidth).arg(m_curtainHeight));
    }
    else {
        painter.drawText(10, 85, "No BMP - Showing heart shape");
    }

    if (m_isFalling) {
        painter.setPen(QColor(0, 255, 0));
        painter.drawText(10, 120, "FALLING - Particles disappearing at bottom");
        if (activeCount == 0) {
            painter.drawText(10, 140, "All particles have fallen! Press SPACE to reset");
        }
    }
    else {
        painter.setPen(QColor(255, 200, 0));
        painter.drawText(10, 120, "STATIC - Press SPACE to start falling");
    }
}