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

    initParticles();

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

        // 打印一些密度样本
        for (int y = 0; y < qMin(3, m_curtainHeight); ++y) {
            for (int x = 0; x < qMin(3, m_curtainWidth); ++x) {
                qDebug() << "Density at" << x << y << ":" << qGray(m_curtainImage.pixel(x, y));
            }
        }
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
        // 默认图案：圆形区域 + 心形效果
        float r = sqrt(x * x + z * z);
        // 心形公式
        float heartX = x;
        float heartZ = z;
        float heartFormula = pow(heartX * heartX + heartZ * heartZ - 1, 3) - heartX * heartX * heartZ * heartZ * heartZ;
        return (r < 3.5f && heartFormula < 0) || (r < 2.0f);
    }

    // 将世界坐标映射到图片坐标
    // 水帘范围：X: -5 到 5, Z: -4 到 4
    float u = (x + 5.0f) / 10.0f;
    float v = (z + 4.0f) / 8.0f;

    if (u < 0 || u > 1 || v < 0 || v > 1) return false;

    int ix = qBound(0, static_cast<int>(u * m_curtainWidth), m_curtainWidth - 1);
    int iy = qBound(0, static_cast<int>(v * m_curtainHeight), m_curtainHeight - 1);

    int gray = qGray(m_curtainImage.pixel(ix, iy));
    float density = gray / 255.0f;

    // 密度越高，生成概率越大
    return QRandomGenerator::global()->generateDouble() < density;
}

void WaterCurtainWidget::initParticles()
{
    m_particles.resize(m_particleCount);

    for (int i = 0; i < m_particleCount; ++i) {
        // 根据BMP决定初始位置
        float x, z;
        int attempts = 0;
        do {
            x = (QRandomGenerator::global()->generateDouble() - 0.5) * 10.0;
            z = (QRandomGenerator::global()->generateDouble() - 0.5) * 8.0;
            attempts++;
            if (attempts > 30) break;
        } while (!shouldSpawnAt(x, z));

        m_particles[i].x = x;
        m_particles[i].z = z;
        m_particles[i].y = -3.0 + QRandomGenerator::global()->generateDouble() * 2.0;

        // 速度：向上为主
        m_particles[i].vx = (QRandomGenerator::global()->generateDouble() - 0.5) * 1.5;
        m_particles[i].vy = 5.0 + QRandomGenerator::global()->generateDouble() * 5.0;
        m_particles[i].vz = (QRandomGenerator::global()->generateDouble() - 0.5) * 1.5;

        m_particles[i].life = 1.0;
    }
}

void WaterCurtainWidget::updateParticles()
{
    float now = QElapsedTimer().nsecsElapsed() / 1e9f;
    float deltaTime = now - m_lastTime;
    m_lastTime = now;

    if (deltaTime > 0.033f) deltaTime = 0.033f;

    float gravity = -12.0f;
    float windX = 0.5f;   // 轻微风力
    float windZ = 0.2f;

    int activeCount = 0;

    for (int i = 0; i < m_particleCount; ++i) {
        Particle& p = m_particles[i];

        // 物理更新
        p.vy += gravity * deltaTime;
        p.vx += windX * deltaTime;
        p.vz += windZ * deltaTime;
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        p.z += p.vz * deltaTime;

        // 边界检查 - 超出范围则重置
        if (p.y < -4.0f || p.y > 7.0f ||
            qAbs(p.x) > 8.0f || qAbs(p.z) > 6.0f) {

            // 根据BMP决定重置位置
            float x, z;
            int attempts = 0;
            do {
                x = (QRandomGenerator::global()->generateDouble() - 0.5) * 10.0;
                z = (QRandomGenerator::global()->generateDouble() - 0.5) * 8.0;
                attempts++;
                if (attempts > 30) break;
            } while (!shouldSpawnAt(x, z));

            p.x = x;
            p.z = z;
            p.y = -3.5;

            p.vx = (QRandomGenerator::global()->generateDouble() - 0.5) * 1.5;
            p.vy = 6.0 + QRandomGenerator::global()->generateDouble() * 6.0;
            p.vz = (QRandomGenerator::global()->generateDouble() - 0.5) * 1.5;
        }

        activeCount++;
    }

    // 每120帧打印一次
    static int frameCount = 0;
    if (++frameCount % 120 == 0) {
        // qDebug() << "Active particles:" << activeCount;
    }

    update();
}

void WaterCurtainWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景（深蓝色渐变）
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(10, 10, 30));
    gradient.setColorAt(1, QColor(20, 30, 50));
    painter.fillRect(rect(), gradient);

    int w = width();
    int h = height();

    // 简单的3D到2D投影
    float fov = 60.0f;
    float aspect = w / (float)h;
    float f = 1.0f / tan(fov * 3.14159f / 360.0f);

    // 相机参数
    float eyeX = 6.0f, eyeY = 4.0f, eyeZ = 8.0f;
    float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
    float upX = 0.0f, upY = 1.0f, upZ = 0.0f;

    // 计算视图矩阵
    float forwardX = centerX - eyeX;
    float forwardY = centerY - eyeY;
    float forwardZ = centerZ - eyeZ;
    float len = sqrt(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
    forwardX /= len; forwardY /= len; forwardZ /= len;

    float rightX = upY * forwardZ - upZ * forwardY;
    float rightY = upZ * forwardX - upX * forwardZ;
    float rightZ = upX * forwardY - upY * forwardX;
    len = sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
    rightX /= len; rightY /= len; rightZ /= len;

    float up2X = forwardY * rightZ - forwardZ * rightY;
    float up2Y = forwardZ * rightX - forwardX * rightZ;
    float up2Z = forwardX * rightY - forwardY * rightX;

    // 投影矩阵
    float near = 0.1f, far = 100.0f;
    float proj[4][4] = {
        {f / aspect, 0, 0, 0},
        {0, f, 0, 0},
        {0, 0, (far + near) / (near - far), (2 * far * near) / (near - far)},
        {0, 0, -1, 0}
    };

    // 绘制所有粒子
    for (const auto& p : m_particles) {
        // 视图变换
        float x = p.x - eyeX;
        float y = p.y - eyeY;
        float z = p.z - eyeZ;

        float viewX = rightX * x + rightY * y + rightZ * z;
        float viewY = up2X * x + up2Y * y + up2Z * z;
        float viewZ = forwardX * x + forwardY * y + forwardZ * z;

        if (viewZ <= 0) continue;

        // 投影变换
        float clipX = proj[0][0] * viewX;
        float clipY = proj[1][1] * viewY;
        float clipW = proj[3][2] * viewZ + proj[3][3];

        float ndcX = clipX / clipW;
        float ndcY = clipY / clipW;

        // 转换到屏幕坐标
        float screenX = (ndcX + 1.0f) * 0.5f * w;
        float screenY = (1.0f - ndcY) * 0.5f * h;

        // 根据高度和位置计算大小和颜色
        float heightFactor = (p.y + 3.0f) / 8.0f;
        float size = 3.0f + heightFactor * 5.0f;

        // 根据XZ位置获取BMP密度（用于颜色）
        float density = 0.5f;
        if (!m_curtainImage.isNull()) {
            float u = (p.x + 5.0f) / 10.0f;
            float v = (p.z + 4.0f) / 8.0f;
            if (u >= 0 && u <= 1 && v >= 0 && v <= 1) {
                int ix = qBound(0, static_cast<int>(u * m_curtainWidth), m_curtainWidth - 1);
                int iy = qBound(0, static_cast<int>(v * m_curtainHeight), m_curtainHeight - 1);
                density = qGray(m_curtainImage.pixel(ix, iy)) / 255.0f;
            }
        }

        // 颜色：根据密度和高度变化
        int r = 80 + density * 100 + heightFactor * 50;
        int g = 100 + density * 100 + heightFactor * 80;
        int b = 180 + density * 75 + heightFactor * 50;

        // 绘制圆形粒子
        painter.setBrush(QColor(r, g, b, 200));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(screenX, screenY), size, size);
    }

    // 绘制信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 30, "Water Curtain Particle System");
    painter.drawText(10, 55, QString("Particles: %1").arg(m_particleCount));

    if (!m_curtainImage.isNull()) {
        painter.drawText(10, 80, QString("BMP Loaded: %1x%2").arg(m_curtainWidth).arg(m_curtainHeight));
    }
    else {
        painter.drawText(10, 80, "No BMP - Using default heart pattern");
        painter.drawText(10, 105, "Place 'water_curtain.bmp' in program folder");
    }
}