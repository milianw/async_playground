#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

struct Foo : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    int a()
    {
        qInfo() << "a runs in" << QThread::currentThreadId();
        QThread::currentThread()->sleep(1);
        qInfo() << "a finished";
        return 42;
    }

    QString b(int i)
    {
        qInfo() << "b runs in" << QThread::currentThreadId() << i;
        QThread::currentThread()->msleep(100);
        qInfo() << "b finished";
        return QString::number(i);
    }

    void c()
    {
        qInfo() << "c runs in" << QThread::currentThreadId();
        QThread::currentThread()->msleep(200);
        qInfo() << "c finished";
    }

    bool d()
    {
        qInfo() << "d runs in" << QThread::currentThreadId();
        QThread::currentThread()->sleep(1);
        qInfo() << "d finished";
        return true;
    }

    QFuture<bool> runTasks()
    {
        return QtConcurrent::run([this]() {
            int i = a();
            auto b_fut = QtConcurrent::run(this, &Foo::b, i);
            auto c_fut = QtConcurrent::run(this, &Foo::c);
            qDebug() << "waiting on b";
            b_fut.waitForFinished();
            qDebug() << "waiting on c";
            c_fut.waitForFinished();
            qDebug() << "running d";
            return d();
        });
    }
};

// dislike: no continuation support
template<typename T>
class WatchedQFuture
{
public:
    WatchedQFuture(const QFuture<T> &future)
        : m_watcher(new QFutureWatcher<T>)
    {
        QObject::connect(m_watcher, &QFutureWatcher<T>::finished, m_watcher, &QObject::deleteLater);
        m_watcher->setFuture(future);
    }

    // dislike: how to continue a continuation...
    // dislike: requires us to actually use the T value in the callback
    template<typename Callback>
    void then(Callback callback)
    {
        if (m_watcher->isFinished()) {
            callback(m_watcher->result());
            return;
        }

        auto future = m_watcher->future();
        QObject::connect(m_watcher, &QFutureWatcher<T>::finished,
                         m_watcher, [future, callback]() {
                            callback(future.result());
                        });
    }
private:
    QFutureWatcher<T> *m_watcher = nullptr;
};

template<typename T>
WatchedQFuture<T> watch(const QFuture<T>& future)
{
    return {future};
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    qInfo() << "main thread is:" << QThread::currentThreadId();

    Foo foo;
    // dislike: no built-in continuation
    watch(foo.runTasks()).then([&app](bool b) {
                                qDebug() << "quitting" << b;
                                app.quit();
                            });

    QTimer t;
    QObject::connect(&t, &QTimer::timeout, &t, []() {
        qInfo() << "handling timer event";
    });
    t.start(100);

    return app.exec();
}

#include "qfuture_kitchensink.moc"
