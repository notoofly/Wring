#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include <QQuickWindow>

#include "wring_controller.hpp"
#include "wring_settings.hpp"
#include "global_input.hpp"

#ifdef Q_OS_LINUX
#include "linux/x11/x11_backend.hpp"
#elif defined(Q_OS_WIN)
#include "windows/windows_backend.hpp"
#endif

Q_LOGGING_CATEGORY(lcMain, "wring.main")

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("wring");
    app.setOrganizationName("wring");
    app.setApplicationVersion("0.1.0");

    qCInfo(lcMain) << "Wring starting...";

    WringController controller;
    WringSettings settings;

#ifdef Q_OS_LINUX
    auto backend = std::make_unique<X11Backend>();
#elif defined(Q_OS_WIN)
    auto backend = std::make_unique<WindowsBackend>();
#else
    qCCritical(lcMain) << "Unsupported platform";
    return 1;
#endif

    if (!backend->initialize()) {
        qCCritical(lcMain) << "Failed to initialize backend";
        return 1;
    }

    controller.setBackend(std::move(backend));

    GlobalInput input(&controller);

    input.setSuperRMBCallback([&controller]() {
        QMetaObject::invokeMethod(&controller, "show", Qt::QueuedConnection);
    });

    input.setSuperRMBReleaseCallback([&controller]() {
        QMetaObject::invokeMethod(&controller, "activateSelected", Qt::QueuedConnection);
    });

    input.setCursorMoveCallback([&controller](int x, int y) {
        QMetaObject::invokeMethod(&controller, "setCursorPosition",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, x), Q_ARG(int, y));
    });

    input.setWheelCallback([&controller](int delta) {
        if (delta > 0) {
            QMetaObject::invokeMethod(&controller, "wheelDown", Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(&controller, "wheelUp", Qt::QueuedConnection);
        }
    });

    if (!input.initialize()) {
        qCCritical(lcMain) << "Failed to initialize input";
        return 1;
    }

    input.startListening();

    input.setTriggerModifier(settings.triggerModifier());
    input.setTriggerButton(settings.triggerButton());

    QObject::connect(&settings, &WringSettings::triggerModifierChanged, &input, [&input, &settings]() {
        input.setTriggerModifier(settings.triggerModifier());
    });
    QObject::connect(&settings, &WringSettings::triggerButtonChanged, &input, [&input, &settings]() {
        input.setTriggerButton(settings.triggerButton());
    });

    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance<WringController>(
        "Wring", 1, 0, "WringController", &controller);
    qmlRegisterSingletonInstance<WringSettings>(
        "Wring", 1, 0, "WringSettings", &settings);
    qCInfo(lcMain) << "Singletons registered";

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        qCCritical(lcMain) << "Failed to load QML";
        return -1;
    }

    qCInfo(lcMain) << "Wring started successfully";

    int result = app.exec();

    input.shutdown();

    qCInfo(lcMain) << "Wring shutting down";
    return result;
}
