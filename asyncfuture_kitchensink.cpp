#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <asyncfuture.h>

using namespace AsyncFuture;

struct Foo : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    int a()
    {
        qDebug() << "start";
        QThread::currentThread()->sleep(1);
        qDebug() << "end";
        return 42;
    }

    QString b(int i)
    {
        qDebug() << "start" << i;
        QThread::currentThread()->msleep(100);
        qDebug() << "end";
        return QString::number(i);
    }

    QString c()
    {
        qDebug() << "start";
        QThread::currentThread()->msleep(200);
        qDebug() << "end";
        return QStringLiteral("Hello World, the answer is: ");
    }

    Deferred<QString> d(const QString& foo, const QString& bar)
    {
        qDebug() << "start" << foo << bar;
        auto ret = deferred<QString>();
        QTimer::singleShot(500, [ret, foo, bar] () mutable {
            qDebug() << "end";
            ret.complete(foo + bar);
        });
        return ret;
    }

    Deferred<QString> runTasks()
    {
        auto ret = deferred<QString>();
        QtConcurrent::run([this, ret] () mutable {
            qDebug() << "runTtasks";
            int i = a();
            qDebug() << "running child tasks";
            auto b_fut = QtConcurrent::run(this, &Foo::b, i);
            auto c_fut = QtConcurrent::run(this, &Foo::c);
            auto childTasks = combine() << b_fut << c_fut;
            childTasks.subscribe([this, c_fut, b_fut, ret] () mutable {
                qDebug() << "child tasks done, finalizing";
                // dislike: cannot complete with deferred value directly
                ret.complete(d(c_fut.result(), b_fut.result()).future());
            });
        });
        return ret;
    }
};

int main(int argc, char** argv)
{
    qputenv("QT_MESSAGE_PATTERN", "%{time process} %{threadid} %{type} %{function} (%{file}:%{line}):\n\t%{message}");
    QCoreApplication app(argc, argv);
    qDebug() << "main thread started";

    // run some task and then eventually quit
    Foo foo;
    foo.runTasks()
        .subscribe([&app](const QString& msg) {
            qDebug() << "got message" << msg;
            app.quit();
        });

    // await two differently typed promises
    auto a = QtConcurrent::run([](){return QStringLiteral("foo");});
    auto b = QtConcurrent::run([](){return 42;});
    (combine() << a << b).subscribe([a, b]() { qDebug() << a.result() << b.result();});

    // run a timer to show that the main thread isn't blocked
    QTimer t;
    QObject::connect(&t, &QTimer::timeout, &t, []() {
        qDebug() << "handling timer event";
    });
    t.start(100);

    return app.exec();
}

#include "asyncfuture_kitchensink.moc"
