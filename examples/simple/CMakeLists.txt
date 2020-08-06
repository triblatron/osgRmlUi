ADD_EXECUTABLE( osgrmlui_simple osgrmlui_simple.cpp )
TARGET_LINK_DIRECTORIES( osgrmlui_simple PUBLIC ${DEP_ROOT}/lib )
TARGET_LINK_LIBRARIES( osgrmlui_simple RmlCore RmlControls RmlDebugger osgRmlUi osgDB osgGA osgUtil )
INSTALL( TARGETS osgrmlui_simple
    RUNTIME DESTINATION share/osgrmlui/bin
)
