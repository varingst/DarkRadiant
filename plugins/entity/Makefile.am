AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libs \
               $(XML_CFLAGS)

modulesdir = $(pkglibdir)/modules
modules_LTLIBRARIES = entity.la

entity_la_LDFLAGS = -module -avoid-version $(GL_LIBS) $(WX_LIBS) $(XML_LIBS)
entity_la_LIBADD = $(top_builddir)/libs/math/libmath.la \
                   $(top_builddir)/libs/xmlutil/libxmlutil.la \
                   $(top_builddir)/libs/scene/libscenegraph.la
entity_la_SOURCES = AngleKey.cpp \
                    RotationKey.cpp \
                    RotationMatrix.cpp \
                    ShaderParms.cpp \
                    ModelKey.cpp \
                    light/Light.cpp \
                    light/Renderables.cpp \
                    light/LightNode.cpp \
                    eclassmodel/EclassModelNode.cpp \
                    EntityCreator.cpp \
                    EntitySettings.cpp \
                    speaker/SpeakerRenderables.cpp \
                    speaker/SpeakerNode.cpp \
                    Doom3Entity.cpp \
                    EntityNode.cpp \
                    doom3group/Doom3Group.cpp \
                    doom3group/Doom3GroupNode.cpp \
                    curve/CurveCatmullRom.cpp \
                    curve/CurveNURBS.cpp \
                    curve/Curve.cpp \
                    curve/CurveEditInstance.cpp \
                    plugin.cpp \
                    generic/GenericEntity.cpp \
                    generic/GenericEntityNode.cpp \
                    NamespaceManager.cpp \
                    KeyValueObserver.cpp \
                    NameKeyObserver.cpp \
                    KeyValue.cpp \
                    target/TargetKey.cpp \
                    target/TargetableNode.cpp \
                    target/TargetLineNode.cpp \
                    target/TargetKeyCollection.cpp \
                    target/TargetManager.cpp

