

#include <fontfactoryinterface.h>

class QFontFactoryFT;

class FreeTypeFactoryImpl : public FontFactoryInterface
{
public:
    FreeTypeFactoryImpl();
    virtual ~FreeTypeFactoryImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QFontFactory *fontFactory();

private:
    QFontFactoryFT *factory;
    ulong ref;
};
