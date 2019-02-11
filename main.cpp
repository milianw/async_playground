#include <QCoreApplication>
#include <thread>
#include <future>
#include <functional>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    return app.exec();
}
