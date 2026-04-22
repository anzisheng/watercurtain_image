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
    setFocusPolicy(Qt::StrongFocus);  // 接收键盘事件

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
    m_isFalling = true;
    m_lastTime = QElapsedTimer().nsecsElapsed() / 1e9f;
}

void WaterCurtainWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space) {
        startFalling();
        qDebug() << "Start falling!";
    }
}

bool WaterCurtainWidget::shouldSpawnAt(float x, float z) const
{
    if (m_curtainImage.isNull()) {
        // 默认心形
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

    // 采样网格，确保形状清晰
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
                p.y = 0.0f;  // 初始在同一平面，便于观察形状
                p.vy = -1.0f - QRandomGenerator::global()->generateDouble() * 1.5f;
                p.active = true;
                m_particles.append(p);
            }
        }
    }

    m_particleCount = m_particles.size();
    qDebug() << "Initialized" << m_particleCount << "particles - Press SPACE to start falling";
}

void WaterCurtainWidget::updateParticles()
{
    if (!m_isFalling) {
        // 未开始下落，不更新物理
        return;
    }

    float now = QElapsedTimer().nsecsElapsed() / 1e9f;
    float deltaTime = now - m_lastTime;
    m_lastTime = now;

    if (deltaTime > 0.033f) deltaTime = 0.033f;
    if (deltaTime < 0.001f) deltaTime = 0.016f;

    float gravity = 8.0f;

    for (int i = 0; i < m_particles.size(); ++i) {
        Particle& p = m_particles[i];

        p.vy += gravity * deltaTime;
        p.y += p.vy * deltaTime;

        // 超出底部重置到顶部
        if (p.y < -3.5f) {
            p.y = 4.0f;
            p.vy = -1.0f - QRandomGenerator::global()->generateDouble() * 1.5f;
        }
    }

    update();
}

void WaterCurtainWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 黑色背景
    painter.fillRect(rect(), QColor(0, 0, 0));

    int w = width();
    int h = height();

    // 正交投影 - 直接显示X-Z平面，像看图片一样
    float marginX = w * 0.1f;
    float marginY = h * 0.1f;
    float drawW = w - 2 * marginX;
    float drawH = h - 2 * marginY;

    // X范围 -5 到 5，Z范围 -4 到 4
    float scaleX = drawW / 10.0f;
    float scaleZ = drawH / 8.0f;

    // 先绘制网格线（帮助定位）
    painter.setPen(QColor(40, 40, 60));
    for (int i = -5; i <= 5; i++) {
        float screenX = marginX + (i + 5) * scaleX;
        painter.drawLine(screenX, marginY, screenX, marginY + drawH);
    }
    for (int i = -4; i <= 4; i++) {
        float screenY = marginY + (4 - i) * scaleZ;
        painter.drawLine(marginX, screenY, marginX + drawW, screenY);
    }

    // 绘制所有粒子 - 显示BMP形状
    for (const auto& p : m_particles) {
        // 屏幕坐标（正交投影）
        float screenX = marginX + (p.x + 5.0f) * scaleX;
        float screenY = marginY + (4.0f - p.z) * scaleZ;

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

        // 根据密度设置大小和颜色
        float size = 3.0f + density * 6.0f;

        // 颜色：密度高 = 白色/青色，密度低 = 蓝色
        int r = 80 + (int)(density * 175);
        int g = 120 + (int)(density * 135);
        int b = 200 + (int)(density * 55);

        // 如果正在下落，根据Y位置调整透明度
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

    // 显示信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(10, 30, "Water Curtain - BMP Shape Display");
    painter.drawText(10, 55, QString("Particles: %1").arg(m_particles.size()));

    if (!m_curtainImage.isNull()) {
        painter.drawText(10, 80, QString("BMP: %1x%2").arg(m_curtainWidth).arg(m_curtainHeight));
    }
    else {
        painter.drawText(10, 80, "No BMP - Showing heart shape");
        painter.drawText(10, 105, "Place 'water_curtain.bmp' in program folder");
    }

    if (m_isFalling) {
        painter.setPen(QColor(0, 255, 0));
        painter.drawText(10, 130, "FALLING MODE - Particles moving down");
    }
    else {
        painter.setPen(QColor(255, 200, 0));
        painter.drawText(10, 130, "STATIC MODE - Press SPACE to start falling");
        painter.drawText(10, 155, "Shape remains clear while falling!");
    }
}