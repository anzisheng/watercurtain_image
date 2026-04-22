#include "MainWindow.h"
#include "WaterCurtainWidget.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_simulationRunning(true)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Water Curtain Particle System - Modern OpenGL");
    resize(1024, 768);
    
    // 创建中央widget和布局
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // 创建OpenGL渲染区域
    m_waterCurtainWidget = new WaterCurtainWidget(this);
    m_waterCurtainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(m_waterCurtainWidget, 4);
    
    // 创建控制面板
    QWidget* controlPanel = new QWidget(this);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    
    // 粒子数量控制
    QGroupBox* particleGroup = new QGroupBox("Particle Settings");
    QVBoxLayout* particleLayout = new QVBoxLayout();
    
    QLabel* countLabel = new QLabel("Particle Count: 50000");
    QSlider* countSlider = new QSlider(Qt::Horizontal);
    countSlider->setRange(1000, 200000);
    countSlider->setValue(50000);
    
    particleLayout->addWidget(countLabel);
    particleLayout->addWidget(countSlider);
    particleGroup->setLayout(particleLayout);
    
    // 控制按钮
    QPushButton* toggleBtn = new QPushButton("Stop");
    QPushButton* loadImageBtn = new QPushButton("Load BMP Image");
    QPushButton* resetBtn = new QPushButton("Reset Camera");
    
    // 密度阈值控制
    QGroupBox* densityGroup = new QGroupBox("Density Threshold");
    QVBoxLayout* densityLayout = new QVBoxLayout();
    QSlider* densitySlider = new QSlider(Qt::Horizontal);
    densitySlider->setRange(0, 255);
    densitySlider->setValue(128);
    QLabel* densityLabel = new QLabel("Threshold: 128");
    densityLayout->addWidget(densityLabel);
    densityLayout->addWidget(densitySlider);
    densityGroup->setLayout(densityLayout);
    
    controlLayout->addWidget(particleGroup);
    controlLayout->addWidget(densityGroup);
    controlLayout->addWidget(toggleBtn);
    controlLayout->addWidget(loadImageBtn);
    controlLayout->addWidget(resetBtn);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlPanel, 1);
    
    setCentralWidget(centralWidget);
    
    // 连接信号槽
    connect(countSlider, &QSlider::valueChanged, this, [this, countLabel](int val) {
        countLabel->setText(QString("Particle Count: %1").arg(val));
        m_waterCurtainWidget->setParticleCount(val);
    });
    
    connect(densitySlider, &QSlider::valueChanged, this, [this, densityLabel](int val) {
        densityLabel->setText(QString("Threshold: %1").arg(val));
        // 可以通过widget暴露接口设置阈值
    });
    
    connect(toggleBtn, &QPushButton::clicked, this, [this, toggleBtn]() {
        if (m_simulationRunning) {
            m_waterCurtainWidget->stopSimulation();
            toggleBtn->setText("Start");
        } else {
            m_waterCurtainWidget->startSimulation();
            toggleBtn->setText("Stop");
        }
        m_simulationRunning = !m_simulationRunning;
    });
    
    connect(loadImageBtn, &QPushButton::clicked, this, &MainWindow::onLoadImage);
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        // 重置相机逻辑
    });
    
    // 启动模拟
    m_waterCurtainWidget->startSimulation();
}

void MainWindow::onLoadImage()
{
    QString filename = QFileDialog::getOpenFileName(this, 
        "Load Water Curtain BMP", 
        QString(),
        "BMP Images (*.bmp);;All Files (*)");
    
    if (!filename.isEmpty()) {
        m_waterCurtainWidget->loadCurtainTexture(filename);
        statusBar()->showMessage("Loaded: " + filename, 3000);
    }
}

void MainWindow::onToggleSimulation()
{
    // 已在上面处理
}

void MainWindow::onParticleCountChanged(int value)
{
    // 已在上面处理
}