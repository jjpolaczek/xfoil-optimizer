TEMPLATE = app

QT += qml quick
CONFIG += c++11

SOURCES += \
    src/gui/view.cpp \
    src/model/model.cpp \
    src/utility/log_writer.cpp \
    src/utility/time_manager.cpp \
    src/main.cpp

RESOURCES += src/gui/qml_files/qml.qrc \
    src/gui/qml_files/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/gui/gui_objects.h \
    src/gui/view.h \
    src/model/model.h \
    src/model/profile_parameters.h \
    src/optimizer/genetic.h \
    src/optimizer/genetic_datatypes.h \
    src/utility/configuration_reader.h \
    src/utility/log_writer.h \
    src/utility/time_manager.h \
    src/utility/utility.h \
    src/xfoil/simulation.h
