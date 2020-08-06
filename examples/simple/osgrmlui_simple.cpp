#include <RmlUi/Core.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Controls.h>
#include <osgRmlUi/RenderInterface>
#include <osgRmlUi/FileInterface>
#include <osgRmlUi/SystemInterface>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/RenderInfo>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osg/Types>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>
#include <osgRmlUi/GuiNode>
#include <osgGA/Device>

#include <iostream>

/// Loads the default fonts from the data path.
void LoadFonts()
{
    Rml::Core::String font_names[5];
    font_names[0] = "Delicious-Roman.otf";
    font_names[1] = "Delicious-Italic.otf";
    font_names[2] = "Delicious-Bold.otf";
    font_names[3] = "Delicious-BoldItalic.otf";
    font_names[4] = "NotoEmoji-Regular.ttf";

    const int fallback_face = 4;


    for (size_t i = 0; i < sizeof(font_names) / sizeof(Rml::Core::String); i++)
    {
        std::string fullPath = osgDB::findDataFile(std::string(font_names[i]));

        if ( !fullPath.empty() )
        {
            Rml::Core::LoadFontFace(fullPath);
        }
        else
        {
            Rml::Core::Log::Message(Rml::Core::Log::LT_WARNING,"Failed to find font %s",font_names[i].c_str());
        }
    }
}

osg::Camera* createHUD()
{
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    // Flip the y axis because RmlUi has the origin in the top-left.
    camera->setProjectionMatrix(osg::Matrix::ortho(0.0,1280.0,1024.0,0.0,-10000.0,10000.0));

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(true);

    return camera;
}

int main( int argc, char * argv[] )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an example of loading an RmlUi document and visualisation of 3D models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>","Provide authentication information for http file access.");
    arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Play specified camera path animation file, previously saved with 'z' key.");
    arguments.getApplicationUsage()->addCommandLineOption("--speed <factor>","Speed factor for animation playing (1 == normal speed).");
    arguments.getApplicationUsage()->addCommandLineOption("--device <device-name>","add named device to the viewer");
    arguments.getApplicationUsage()->addCommandLineOption("--stats","print out load and compile timing stats");
    arguments.getApplicationUsage()->addCommandLineOption("--document <fllename>", "Load document");

    std::string documentFilename;
    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer(arguments);
    viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    osgRmlUi::FileInterface FileInterface;
    osgRmlUi::SystemInterface SystemInterface;
    Rml::Core::SetFileInterface(&FileInterface);
    Rml::Core::SetSystemInterface(&SystemInterface);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    bool printStats = arguments.read("--stats");

    std::string url, username, password;
    while(arguments.read("--login",url, username, password))
    {
        osgDB::Registry::instance()->getOrCreateAuthenticationMap()->addAuthenticationDetails(
            url,
            new osgDB::AuthenticationDetails(username, password)
        );
    }

    std::string device;
    while(arguments.read("--device", device))
    {
        osg::ref_ptr<osgGA::Device> dev = osgDB::readRefFile<osgGA::Device>(device);
        if (dev.valid())
        {
            viewer->addDevice(dev);
        }
    }
/*
    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
        keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
        keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
        keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );

        std::string pathfile;
        double animationSpeed = 1.0;
        while(arguments.read("--speed",animationSpeed) ) {}
        char keyForAnimationPath = '8';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm && !apm->getAnimationPath()->empty())
            {
                apm->setTimeScale(animationSpeed);

                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer->setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the state manipulator
    viewer->addEventHandler( new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    viewer->addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer->addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    viewer->addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the record camera path handler
    viewer->addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    viewer->addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    viewer->addEventHandler(new osgViewer::ScreenCaptureHandler);
*/
    osg::ElapsedTime elapsedTime;

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    if (printStats)
    {
        double loadTime = elapsedTime.elapsedTime_m();
        std::cout<<"Load time "<<loadTime<<"ms"<<std::endl;

        viewer->getStats()->collectStats("compile", true);
    }


    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel);

    viewer->setUpViewAcrossAllScreens();
    osgViewer::Viewer::Windows windows;
    viewer->getWindows(windows);

    if (windows.empty()) return 1;

    osg::Camera* hudCamera = createHUD();

    // set up cameras to render on the first window available.
    hudCamera->setGraphicsContext(windows[0]);
    hudCamera->setViewport(0,0,windows[0]->getTraits()->width, windows[0]->getTraits()->height);
    viewer->addSlave(hudCamera,false);
    viewer->setSceneData(loadedModel);
    viewer->realize();

    osgRmlUi::RenderInterface * Renderer = new osgRmlUi::RenderInterface();
    Rml::Core::SetRenderInterface(Renderer);
    Rml::Core::Initialise();
    Rml::Controls::Initialise();
    osg::ref_ptr<osgRmlUi::GuiNode> renderTarget = new osgRmlUi::GuiNode("default", false);
    renderTarget->setCamera(hudCamera);
    osg::ref_ptr<osg::Group> scene = hudCamera;
    Renderer->setRenderTarget(renderTarget, windows[0]->getTraits()->width, windows[0]->getTraits()->height, true);
    scene->addChild(renderTarget);
    LoadFonts();
    Rml::Debugger::Initialise(renderTarget->getContext());

    Rml::Core::ElementDocument * doc = 0;
    if ( arguments.read("--document", documentFilename) )
    {
        doc = renderTarget->getContext()->LoadDocument(documentFilename.c_str());
    }
    viewer->addEventHandler( renderTarget->GetGUIEventHandler() );
    if ( doc )
    {
        doc->Show();
    }

    while ( !viewer->done() )
    {
        viewer->frame();
    }
    Rml::Core::Shutdown();
}
