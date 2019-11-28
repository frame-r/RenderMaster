
# https://evileg.com/ru/post/164/

TEMPLATE = aux


INSTALLER_OFFLINE = $$PWD/../../../bin/Hello.offline

DESTDIR_WIN = $$PWD/packages/xx.framer.engine/data
DESTDIR_WIN ~= s,/,\\,g
PWD_WIN = $$PWD/RenderMaster/../../bin
PWD_WIN ~= s,/,\\,g

QMAKE_EXTRA_TARGETS += first copydata


INPUT = $$PWD/config/config.xml $$PWD/packages
offlineInstaller.depends = copydata
offlineInstaller.input = INPUT
offlineInstaller.output = $$INSTALLER_OFFLINE
offlineInstaller.commands = $$(QTDIR)/../../../QtIFW-3.1.1/bin/binarycreator -v --offline-only -c $$PWD/config/config.xml -p $$PWD/packages ../../bin/RenderMasterInstaller_v0.1
offlineInstaller.CONFIG += target_predeps no_link combine

QMAKE_EXTRA_COMPILERS += offlineInstaller


##CONFIG(release, debug|release) {
##    QMAKE_POST_LINK += C:/Qt/QtIFW-3.1.1/bin/repogen -v -p $$PWD/packages -i xx.framer.helloinstaller --update $$OUT_PWD/../../repository
##}

DISTFILES += \
    packages/xx.framer.engine/meta/package.xml \
    config/config.xml
