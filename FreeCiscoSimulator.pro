QT += network
HEADERS = mainwindow.h \
    diagramitem.h \
    diagramscene.h \
    link.h \
    Device.h \
    global_variables.h \
    PC.h \
    console.h \
    router.h \
    deviceport.h \
    command.h \
    ipcommand.h \
    devicecommand.h \
    routeripconfigurecommand.h \
    routeriproutecommand.h \
    routershowcommand.h
SOURCES = mainwindow.cpp \
    diagramitem.cpp \
    diagramscene.cpp \
    main.cpp \
    link.cpp \
    Device.cpp \
    PC.cpp \
    console.cpp \
    router.cpp \
    deviceport.cpp \
    ipcommand.cpp \
    devicecommand.cpp \
    routeripconfigurecommand.cpp \
    routeriproutecommand.cpp \
    routershowcommand.cpp
RESOURCES = FreeCiscoSimulator.qrc
TRANSLATIONS = translation_ru.ts
