#pragma once

#include <QObject>
#include <vector>

struct Component
{
    QObject *object;
    std::string name;
};

struct GuiObjects
{
    QObject* mainWindow;
    Component settingsButtonsRoot;
    Component baseParametersRoot;
    Component targetValuesRoot;
    Component fitnessValuesRoot;

    Component baseChartFrame;
    Component optimizeChartFrame;
    Component busyIndicator;

    QObject* basePlot;
    QObject* optimizedPlot;

    std::vector<QObject*> settingsButtons;
    std::vector<QObject*> baseParameters;
    std::vector<QObject*> targetValues;
    std::vector<QObject*> fitnessValues;

    const int buttonsCount = 3;
    const int parametersLabelCount = 3;

    bool SET_TARGET = false;
    bool SET_BASE = false;
};
