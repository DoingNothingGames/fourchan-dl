
#include "application.h"

#if QT_VERSION < 0x050000
#include <QCleanlooksStyle>
#endif

#if QT_VERSION < 0x040000
#error "Sorry mate, this application needs Qt4.x.x to run properly."
#endif

int main(int argc, char* argv[])
{
  chandl::Application app(argc, argv);

  return app.exec();
}
