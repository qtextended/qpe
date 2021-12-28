#include "fontfactoryttf_qws.h"
#include "freetypefactoryimpl.h"


FreeTypeFactoryImpl::FreeTypeFactoryImpl()
    : factory(0), ref(0)
{
}

FreeTypeFactoryImpl::~FreeTypeFactoryImpl()
{
    delete factory;
}

QFontFactory *FreeTypeFactoryImpl::fontFactory()
{
    if ( !factory )
	factory = new QFontFactoryFT();
    return factory;
}

QRESULT FreeTypeFactoryImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_FontFactory )
	*iface = this;

    if ( *iface )
	(*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( FreeTypeFactoryImpl )
}
