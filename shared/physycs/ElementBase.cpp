#include "ElementBase.h"

ElementBase::ElementBase( b2World* world, const b2BodyDef& bd, const b2FixtureDef& fd ) :   _body()
{
    _body = world->CreateBody( &bd );
    _body->CreateFixture( &fd );

    getBody()->SetUserData(this);
}

ElementBase::ElementBase(b2World* world, const b2BodyDef& bd, const std::vector<b2FixtureDef>& fds) :   _body()
{
    _body = world->CreateBody( &bd );

    for (std::vector<b2FixtureDef>::const_iterator i = fds.begin(); i != fds.end(); ++i) {
       _body->CreateFixture( &*i );
    }

    getBody()->SetUserData(this);
}

ElementBase::ElementBase( b2World* world, const b2BodyDef& bd ) :   _body()
{
    _body = world->CreateBody( &bd );

    getBody()->SetUserData(NULL);
}

ElementBase::~ElementBase() {
    _body->GetWorld()->DestroyBody( _body );
}
