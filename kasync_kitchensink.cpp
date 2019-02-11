#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <KAsync/Async>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

struct Foo : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    bool a()
    {
        qInfo() << "a runs in" << QThread::currentThreadId();
        QThread::currentThread()->sleep(1);
        qInfo() << "a finished";
        return true;
    }

    // dislike: QFuture not unwrapped
    KAsync::Future<QFuture<bool>> runTasks()
    {
        // dislike: have to specify the return type
        qDebug() << "start !";
        return KAsync::start<QFuture<bool>>([this](){
            qDebug() << "start a";
            auto ret = QtConcurrent::run(this, &Foo::a);
            qDebug() << "done starting a";
            return ret;
        }).exec(); // dislike: must not forget to start jobs
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    qInfo() << "main thread is:" << QThread::currentThreadId();

    Foo foo;
    // dislike: complex to continue a future
    // WTF: this compiles but doesn't block for the future?
    KAsync::start<void, KAsync::Future<bool>>([&app](const KAsync::Future<bool>& foo) {
        qInfo() << "quitting";
        app.quit();
        // dislike: have to return null?
        return KAsync::null<void>();
    }).exec(foo.runTasks());

    QTimer t;
    QObject::connect(&t, &QTimer::timeout, &t, []() {
        qInfo() << "handling timer event";
    });
    t.start(100);

    return app.exec();
}

#include "kasync_kitchensink.moc"
