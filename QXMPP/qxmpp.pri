QT += network xml core

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

unix:!macx: LIBS += -lcryptopp

HEADERS += \
  $$PWD/SASL/sasl_base.h \
  $$PWD/SASL/sasl_digest_md5.h \
  $$PWD/SASL/sasl_plain.h \
  $$PWD/SASL/sasl_scram_sha1.h \
  $$PWD/jabber.h \
  $$PWD/jid.h \
  $$PWD/package.h \
  $$PWD/transport.h

SOURCES += \
  $$PWD/SASL/sasl_base.cpp \
  $$PWD/SASL/sasl_digest_md5.cpp \
  $$PWD/SASL/sasl_plain.cpp \
  $$PWD/SASL/sasl_scram_sha1.cpp \
  $$PWD/jabber.cpp \
  $$PWD/package.cpp \
  $$PWD/transport.cpp
