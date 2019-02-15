#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QtPromise>

using namespace QtPromise;

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

    QPromise<QString> d(const QString& foo, const QString& bar)
    {
        // dislike: a bit verbose to connect signals
        //          cf.: https://github.com/simonbrunel/qtpromise/issues/11
        return QPromise<QString>([foo, bar](
            const QPromiseResolve<QString>& resolve,
            const QPromiseReject<QString>& /*reject*/) {
            // simulate async signal handling
            QTimer::singleShot(500, [foo, bar, resolve]() {
                qDebug() << "d timer fired";
                resolve(foo + bar);
                qDebug() << "d timer fired done";
            });
        });
    }

    QPromise<QString> runTasks()
    {
        // dislike: cannot return QPromise from background task
        //          cf.: https://github.com/simonbrunel/qtpromise/issues/22
        return qPromise(QtConcurrent::run([this]() {
            int i = a();
            auto b_fut = QtConcurrent::run(this, &Foo::b, i);
            auto c_fut = QtConcurrent::run(this, &Foo::c);
            qDebug() << "waiting on b";
            b_fut.waitForFinished();
            qDebug() << "waiting on c";
            c_fut.waitForFinished();
            qDebug() << "running d";
            return qMakePair(c_fut.result(), b_fut.result());
        })).then([this](const QPair<QString, QString>& args) {
            return d(args.first, args.second);
        });
    }
};

// cf.: https://github.com/simonbrunel/qtpromise/issues/23
template<typename T1, typename T2>
QPromise<QPair<T1, T2>> qAwaitAll(const QPromise<T1>& p1, const QPromise<T2>& p2)
{
    return p1.then([p2](const T1& arg1) {
        return p2.then([arg1](const T2& arg2) {
            return QPromise<QPair<T1, T2>>([arg1, arg2](const QPromiseResolve<QPair<T1, T2>>& resolve){
                resolve(qMakePair(arg1, arg2));
            });
        });
    });
}

int main(int argc, char** argv)
{
    qputenv("QT_MESSAGE_PATTERN", "%{time process} %{threadid} %{type} %{function} (%{file}:%{line}):\n\t%{message}");
    QCoreApplication app(argc, argv);
    qDebug() << "main thread started";

    // run some task and then eventually quit
    Foo foo;
    foo.runTasks()
        .then([](const QString& msg) {
            qDebug() << "got message" << msg;
        })
        .finally([&app](){
            app.quit();
        });

    // await two differently typed promises
    auto a = qPromise(QtConcurrent::run([](){return QStringLiteral("foo");}));
    auto b = qPromise(QtConcurrent::run([](){return 42;}));
    qAwaitAll(a, b).then([](const QPair<QString, int>& data) { qDebug() << data;});

    // run a timer to show that the main thread isn't blocked
    QTimer t;
    QObject::connect(&t, &QTimer::timeout, &t, []() {
        qDebug() << "handling timer event";
    });
    t.start(100);

    return app.exec();
}

#include "qpromise_kitchensink.moc"
