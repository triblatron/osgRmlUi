/*  , an interface for OpenSceneGraph to use RmlUI
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/
//
// This code is copyright (c) 2011 Martin Scheffler martin.scheffler@googlemail.com
//

#include <osgRmlUi/RenderInterface>
#include <osg/BlendFunc>
#include <osg/MatrixTransform>
#include <assert.h>
#include <osg/Geode>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

//#define USE_SHADERS
#ifdef USE_SHADERS

namespace osgRmlUi
{
    std::string uniformName = "RmlUIPosition";

    RenderInterface::RenderInterface()
        : _scissorsEnabled(false)
        , _fullScreen(false)
        , _geode(new osg::Geode())
        , _renderTarget(NULL)
    {
        _geode->setDataVariance(osg::Object::DYNAMIC);
    }


    void RenderInterface::setRenderTarget(osg::Group* grp, int w, int h, bool fullscreen)
    {
        _fullScreen = fullscreen;
        _screenWidth = w;
        _screenHeight = h;

        if(_renderTarget != grp)
        {
             if(_renderTarget != NULL)
             {
                 _renderTarget->removeChild(_geode);
             }
             _renderTarget = grp;
             _renderTarget->addChild(_geode);
             _renderTargetStateSet = grp->getOrCreateStateSet();
             _renderTargetStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
             _renderTargetStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
             _renderTargetStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

            /* std::string shadertxt = ""
                     "#version 330\n"
                     "uniform vec2 RmlUIPosition;\n"
                     "in vec4 osg_Vertex;\n"
                     "uniform mat4 osg_ModelViewProjectionMatrix;\n"
                     "in vec4 osg_MultiTexCoord0;\n"
                     "in vec4 osg_Color;\n"
                     "varying out vec4 texCoord;\n"
                     "out vec4 out_color;\n"
                     "\n"
                     "void main() {\n"
                     "  texCoord = osg_MultiTexCoord0;\n"
                     "  vec4 pos = osg_Vertex + vec4(RmlUIPosition, 0, 0);\n"
                     "  gl_Position = osg_ModelViewProjectionMatrix * pos;\n"
                     "  out_color = osg_Color;\n"
                     "} \n";*/
             std::string shadertxt = ""
                                  "uniform vec2 RmlUIPosition;\n"
                                  "varying vec2 texCoord;\n"
                                  "varying vec4 out_color;\n"
                                  "\n"
                                  "void main() {\n"
                                  "  texCoord = vec2(gl_MultiTexCoord0);\n"
                                  "  vec4 pos = gl_Vertex + vec4(RmlUIPosition, 0, 0);\n"
                                  "  gl_Position = gl_ModelViewProjectionMatrix * pos;\n"
                                  "  out_color = gl_Color;\n"
                                  "} \n";
             osg::Shader* shader = new osg::Shader(osg::Shader::VERTEX, shadertxt);
             osg::Program* prg = new osg::Program();
             prg->addShader(shader);

             std::string fshadertxt = ""
                     "uniform sampler2D diffuseTexture;\n"
                     "in vec2 texCoord;"
                     "in vec4 out_color;"
                     "\n"
                     "void main() { \n"
                        "gl_FragColor = out_color * texture2D(diffuseTexture, texCoord);\n"
                     "}\n";
             osg::Shader* fshader = new osg::Shader(osg::Shader::FRAGMENT, fshadertxt);
             prg->addShader(fshader);
             _geode->getOrCreateStateSet()->setAttributeAndModes(prg);
        }

        /*for(unsigned int i = 0; i < _geode->getNumDrawables(); ++i)
        {
            osg::Geometry* geom = static_cast<osg::Geometry*>(_geode->getDrawable(i));
            assert(geom->getNumPrimitiveSets() == 1);
            geom->getPrimitiveSet(0)->setNumInstances(0);
            osg::StateSet* ss = geom->getOrCreateStateSet();
        }*/
        _geode->removeDrawables(0, _geode->getNumDrawables());
    }


    osg::Group* RenderInterface::getRenderTarget() const
    {
        return _renderTarget;
    }


    osg::Object* RenderInterface::createGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture, bool useVBOs)
    {

        osg::Geometry* geometry = new osg::Geometry();
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(useVBOs);
        geometry->setDataVariance(osg::Object::DYNAMIC);

        osg::Vec3Array* vertarray = new osg::Vec3Array(num_vertices);
        osg::Vec4Array* colorarray = new osg::Vec4Array(num_vertices);
        osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(num_vertices);

        for(int i = 0; i < num_vertices; ++i)
        {
            Rml::Core::Vertex* vert = &vertices[i];
            Rml::Core::Colourb c = vert->colour;
            (*vertarray)[i].set(vert->position.x, vert->position.y, 0);
            (*colorarray)[i].set(c.red / 255.0f, c.green / 255.0f, c.blue / 255.0f, c.alpha / 255.0f);
            (*texcoords)[i].set(vert->tex_coord.x, vert->tex_coord.y);
        }

#ifdef _ANDROID_
      osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES, num_indices);
      for(int i = 0; i < num_indices; ++i)
      {
          elements->setElement(i, indices[i]);
      }
#else
      osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, num_indices, (const GLuint*)indices, 0);
#endif

        geometry->setVertexArray(vertarray);
        geometry->setColorArray(colorarray);
        geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        geometry->addPrimitiveSet(elements);

        if(texture != 0)
        {
            osg::Texture* tex = reinterpret_cast<osg::Texture*>(texture);
            geometry->setTexCoordArray(0, texcoords);
            osg::StateSet* ss = geometry->getOrCreateStateSet();
            ss->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
        }

        return geometry;
    }


    /// Called by RmlUI when it wants to render geometry that the application does not wish to optimise. Note that
    /// RmlUI renders everything as triangles.
    /// @param[in] vertices The geometry's vertex data.
    /// @param[in] num_vertices The number of vertices passed to the function.
    /// @param[in] indices The geometry's index data.
    /// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
    /// @param[in] texture The texture to be applied to the geometry. This may be NULL, in which case the geometry is untextured.
    /// @param[in] translation The translation to apply to the geometry.
    void RenderInterface::RenderGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture, const Rml::Core::Vector2f& translation)
    {

        osg::Geometry* geometry = static_cast<osg::Geometry*>(createGeometry(vertices, num_vertices, indices, num_indices, texture, false));

        osg::MatrixTransform* trans = new osg::MatrixTransform();
        trans->setMatrix(osg::Matrix::translate(osg::Vec3(translation.x, translation.y, 0)));
        osg::Geode* g = new osg::Geode();
        g->addDrawable(geometry);
        trans->addChild(g);

      if(_scissorsEnabled)
      {
         geometry->getOrCreateStateSet()->setAttributeAndModes(_scissorTest, osg::StateAttribute::ON);
      }

      _renderTarget->addChild(trans);

        _instantGeometryMap.push_back(trans);
    }


    /// Called by RmlUI when it wants to compile geometry it believes will be static for the forseeable future.
    /// If supported, this should be return a pointer to an optimised, application-specific version of the data. If
    /// not, do not override the function or return NULL; the simpler RenderGeometry() will be called instead.
    /// @param[in] vertices The geometry's vertex data.
    /// @param[in] num_vertices The number of vertices passed to the function.
    /// @param[in] indices The geometry's index data.
    /// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
    /// @param[in] texture The texture to be applied to the geometry. This may be NULL, in which case the geometry is untextured.
    /// @return The application-specific compiled geometry. Compiled geometry will be stored and rendered using RenderCompiledGeometry() in future calls, and released with ReleaseCompiledGeometry() when it is no longer needed.
    Rml::Core::CompiledGeometryHandle RenderInterface::CompileGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture)
    {
        osg::Geometry* node = static_cast<osg::Geometry*>(createGeometry(vertices, num_vertices, indices, num_indices, texture, true));
        node->ref();
        osg::Uniform* posuni = new osg::Uniform(osg::Uniform::FLOAT_VEC2, uniformName);
        node->getOrCreateStateSet()->addUniform(posuni);
        return reinterpret_cast<Rml::Core::CompiledGeometryHandle>(node);
    }


    /// Called by RmlUI when it wants to render application-compiled geometry.
    /// @param[in] geometry The application-specific compiled geometry to render.
    /// @param[in] translation The translation to apply to the geometry.
    void RenderInterface::RenderCompiledGeometry(Rml::Core::CompiledGeometryHandle geo, const Rml::Core::Vector2f& translation)
    {

        osg::Geometry* geometry = reinterpret_cast<osg::Geometry*>(geo);

        osg::StateSet* ss = geometry->getOrCreateStateSet();

      osg::Uniform* uni = ss->getUniform(uniformName);
      assert(uni);
      assert(uni->getType() == osg::Uniform::FLOAT_VEC2);
      uni->set(osg::Vec2(translation.x, translation.y));

        /*
        if(_scissorsEnabled)
        {
            node->getOrCreateStateSet()->setAttributeAndModes(_scissorTest, osg::StateAttribute::ON);
        }
        else
        {
            osg::StateSet* ss = node->getStateSet();
            if(ss)
            {
                node->getOrCreateStateSet()->removeAttribute(_scissorTest);
            }
        }*/

        if(geometry->getNumParents() == 0)
        {
            _geode->addDrawable(geometry);
        }

    }


    /// Called by RmlUI when it wants to release application-compiled geometry.
    /// @param[in] geometry The application-specific compiled geometry to release.
    void RenderInterface::ReleaseCompiledGeometry(Rml::Core::CompiledGeometryHandle geo)
    {
        osg::Geometry* geometry = reinterpret_cast<osg::Geometry*>(geo);

        if(geometry->getNumParents() != 0)
        {
            assert(geometry->getNumParents() == 1);
            _geode->removeDrawable(geometry);
            assert(geometry->getNumParents() == 0);
        }
        geometry->unref();
    }


    /// Called by RmlUI when it wants to enable or disable scissoring to clip content.
    /// @param[in] enable True if scissoring is to enabled, false if it is to be disabled.
    void RenderInterface::EnableScissorRegion(bool enable)
    {
        // cannot use scissors when rendering to in-scene geometry
        if(_fullScreen)
        {
            _scissorsEnabled = enable;
        }
    }


    /// Called by RmlUI when it wants to change the scissor region.
    /// @param[in] x The left-most pixel to be rendered. All pixels to the left of this should be clipped.
    /// @param[in] y The top-most pixel to be rendered. All pixels to the top of this should be clipped.
    /// @param[in] width The width of the scissored region. All pixels to the right of (x + width) should be clipped.
    /// @param[in] height The height of the scissored region. All pixels to below (y + height) should be clipped.
    void RenderInterface::SetScissorRegion(int x, int y, int width, int height)
    {
        _scissorTest = new osg::Scissor(x, _screenHeight - y - height, width, height);
    }


    void RenderInterface::AddTexture(Rml::Core::TextureHandle& texture_handle, osg::Image* image)
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
        texture->setResizeNonPowerOfTwoHint(false);
        texture->setImage(image);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        if(_fullScreen)
        {
            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
            texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        }
        else
        {
            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        }
        texture_handle = reinterpret_cast<Rml::Core::TextureHandle>(texture.get());
        texture->ref();
    }


    /// Called by RmlUI when a texture is required by the library.
    /// @param[out] texture_handle The handle to write the texture handle for the loaded texture to.
    /// @param[out] texture_dimensions The variable to write the dimensions of the loaded texture.
    /// @param[in] source The application-defined image source, joined with the path of the referencing document.
    /// @return True if the load attempt succeeded and the handle and dimensions are valid, false if not.
    bool RenderInterface::LoadTexture(Rml::Core::TextureHandle& texture_handle, Rml::Core::Vector2i& texture_dimensions, const Rml::Core::String& source)
    {

        std::string src = source.CString();
        if(src.substr(0, 10) != "RmlUI/")
        {
            src = "RmlUI/" + src;
        }

        std::string path = osgDB::findDataFile(src);
        if(path.empty())
        {
            return false;
        }
        osg::ref_ptr<osg::Image> img = osgDB::readImageFile(path);
        if(!img.valid())
        {
            return false;
        }
        img->flipVertical();

        if(img == NULL) return false;

        texture_dimensions.x = img->s();
        texture_dimensions.y = img->t();

        AddTexture(texture_handle, img);

        return true;
    }


    /// Called by RmlUI when a texture is required to be built from an internally-generated sequence of pixels.
    /// @param[out] texture_handle The handle to write the texture handle for the generated texture to.
    /// @param[in] source The raw 8-bit texture data. Each pixel is made up of four 8-bit values, indicating red, green, blue and alpha in that order.
    /// @param[in] source_dimensions The dimensions, in pixels, of the source data.
    /// @return True if the texture generation succeeded and the handle is valid, false if not.
    bool RenderInterface::GenerateTexture(Rml::Core::TextureHandle& texture_handle, const Rml::Core::byte* source, const Rml::Core::Vector2i& source_dimensions)
    {

        osg::ref_ptr<osg::Image> img = new osg::Image();
        int w = source_dimensions.x;
        int h = source_dimensions.y;
        img->allocateImage(w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        memcpy(img->data(), source, w * h * 4 * sizeof(Rml::Core::byte));

        AddTexture(texture_handle, img);
        return true;
    }


    /// Called by RmlUI when a loaded texture is no longer required.
    /// @param texture The texture handle to release.
    void RenderInterface::ReleaseTexture(Rml::Core::TextureHandle th)
    {
        osg::Texture2D* texture = reinterpret_cast<osg::Texture2D*>(th);
        texture->unref();
    }


    /// Called when this render interface is released.
    void RenderInterface::Release()
    {

    }

}

#else

namespace osgRmlUi
{

    RenderInterface::RenderInterface()
        : _scissorsEnabled(false)
        , _fullScreen(false)
    {
    }

    void RenderInterface::setRenderTarget(osg::Group* grp, int w, int h, bool fullscreen)
    {
        _fullScreen = fullscreen;
        _screenWidth = w;
        _screenHeight = h;

      unsigned int i = 0;
      while(i < grp->getNumChildren())
      {
         osg::Node* node = grp->getChild(i);

         if(node->referenceCount() == 2)
         {
            grp->removeChild(i);
         }
         else
         {
            node->unref();
            ++i;
         }
      }

        if(_renderTarget != grp)
        {
             _renderTarget = grp;
             _renderTargetStateSet = grp->getOrCreateStateSet();
             _renderTargetStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
             _renderTargetStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
             _renderTargetStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        }
    }

    osg::Group* RenderInterface::getRenderTarget() const
    {
        return _renderTarget;
    }

    osg::Object* RenderInterface::createGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture, bool useVBO)
    {

        osg::Geometry* geometry = new osg::Geometry();
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(useVBO);
        geometry->setDataVariance(osg::Object::DYNAMIC);

        osg::Vec3Array* vertarray = new osg::Vec3Array(num_vertices);
        osg::Vec4Array* colorarray = new osg::Vec4Array(num_vertices);
        osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(num_vertices);

        for(int i = 0; i < num_vertices; ++i)
        {
            Rml::Core::Vertex* vert = &vertices[i];
            Rml::Core::Colourb c = vert->colour;
            (*vertarray)[i].set(vert->position.x, vert->position.y, 0);
            (*colorarray)[i].set(c.red / 255.0f, c.green / 255.0f, c.blue / 255.0f, c.alpha / 255.0f);
            (*texcoords)[i].set(vert->tex_coord.x, vert->tex_coord.y);
        }

#ifdef _ANDROID_
      osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES, num_indices);
      for(int i = 0; i < num_indices; ++i)
      {
          elements->setElement(i, indices[i]);
      }
#else
      osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, num_indices, (const GLuint*)indices, 0);
#endif
        geometry->setVertexArray(vertarray);
        geometry->setColorArray(colorarray);
        geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        geometry->addPrimitiveSet(elements);

        if(texture != 0)
        {
            osg::StateSet* ss = reinterpret_cast<osg::StateSet*>(texture);
            geometry->setTexCoordArray(0, texcoords.get());
            geometry->setStateSet(ss);
        }


      osg::Geode* geode = new osg::Geode();
      geode->setCullingActive(false);
      geode->setDataVariance(osg::Object::DYNAMIC);
      geode->addDrawable(geometry);
      osg::MatrixTransform* mt = new osg::MatrixTransform();
      mt->addChild(geode);
      mt->setCullingActive(false);
      mt->setDataVariance(osg::Object::DYNAMIC);
      return mt;
   }


    /// Called by RmlUI when it wants to render geometry that the application does not wish to optimise. Note that
    /// RmlUI renders everything as triangles.
    /// @param[in] vertices The geometry's vertex data.
    /// @param[in] num_vertices The number of vertices passed to the function.
    /// @param[in] indices The geometry's index data.
    /// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
    /// @param[in] texture The texture to be applied to the geometry. This may be NULL, in which case the geometry is untextured.
    /// @param[in] translation The translation to apply to the geometry.
    void RenderInterface::RenderGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture, const Rml::Core::Vector2f& translation)
    {

        osg::MatrixTransform* trans = static_cast<osg::MatrixTransform*>(createGeometry(vertices, num_vertices, indices, num_indices, texture, false));

        trans->setMatrix(osg::Matrix::translate(osg::Vec3(translation.x, translation.y, 0)));

      if(_scissorsEnabled)
      {
         trans->getOrCreateStateSet()->setAttributeAndModes(_scissorTest.get(), osg::StateAttribute::ON);
      }
      _renderTarget->addChild(trans);
      //_renderTarget->dirtyBound();

        _instantGeometryMap.push_back(trans);
    }

    /// Called by RmlUI when it wants to compile geometry it believes will be static for the forseeable future.
    /// If supported, this should be return a pointer to an optimised, application-specific version of the data. If
    /// not, do not override the function or return NULL; the simpler RenderGeometry() will be called instead.
    /// @param[in] vertices The geometry's vertex data.
    /// @param[in] num_vertices The number of vertices passed to the function.
    /// @param[in] indices The geometry's index data.
    /// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
    /// @param[in] texture The texture to be applied to the geometry. This may be NULL, in which case the geometry is untextured.
    /// @return The application-specific compiled geometry. Compiled geometry will be stored and rendered using RenderCompiledGeometry() in future calls, and released with ReleaseCompiledGeometry() when it is no longer needed.
    Rml::Core::CompiledGeometryHandle RenderInterface::CompileGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture)
    {
        osg::Object* node = createGeometry(vertices, num_vertices, indices, num_indices, texture, true);
        node->ref();
        return reinterpret_cast<Rml::Core::CompiledGeometryHandle>(node);
    }

    /// Called by RmlUI when it wants to render application-compiled geometry.
    /// @param[in] geometry The application-specific compiled geometry to render.
    /// @param[in] translation The translation to apply to the geometry.
    void RenderInterface::RenderCompiledGeometry(Rml::Core::CompiledGeometryHandle geometry, const Rml::Core::Vector2f& translation)
    {

        osg::MatrixTransform* trans = reinterpret_cast<osg::MatrixTransform*>(geometry);
        trans->setMatrix(osg::Matrix::translate(osg::Vec3(translation.x, translation.y, 0)));
        trans->getOrCreateStateSet()->setAttributeAndModes(_scissorTest.get(), _scissorsEnabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

      if(trans->referenceCount() == 1)
      {
         _renderTarget->addChild(trans);
      }
      trans->ref();
   }

    /// Called by RmlUI when it wants to release application-compiled geometry.
    /// @param[in] geometry The application-specific compiled geometry to release.
    void RenderInterface::ReleaseCompiledGeometry(Rml::Core::CompiledGeometryHandle geometry)
    {
        osg::Node* node = reinterpret_cast<osg::Node*>(geometry);
        while(node->getNumParents() != 0)
        {
            node->getParent(0)->removeChild(node);
        }
        unsigned int refcount = node->referenceCount();
        while(refcount > 0)
        {
            node->unref();
            --refcount;
        }
    }

    /// Called by RmlUI when it wants to enable or disable scissoring to clip content.
    /// @param[in] enable True if scissoring is to enabled, false if it is to be disabled.
    void RenderInterface::EnableScissorRegion(bool enable)
    {
        // cannot use scissors when rendering to in-scene geometry
        if(_fullScreen)
        {
            _scissorsEnabled = enable;
        }
    }

    /// Called by RmlUI when it wants to change the scissor region.
    /// @param[in] x The left-most pixel to be rendered. All pixels to the left of this should be clipped.
    /// @param[in] y The top-most pixel to be rendered. All pixels to the top of this should be clipped.
    /// @param[in] width The width of the scissored region. All pixels to the right of (x + width) should be clipped.
    /// @param[in] height The height of the scissored region. All pixels to below (y + height) should be clipped.
    void RenderInterface::SetScissorRegion(int x, int y, int width, int height)
    {
        _scissorTest = new osg::Scissor(x, _screenHeight - y - height, width, height);
    }

    void RenderInterface::AddTexture(Rml::Core::TextureHandle& texture_handle, osg::Image* image)
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
        texture->setResizeNonPowerOfTwoHint(false);
        texture->setImage(image);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        if(_fullScreen)
        {
            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
            texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        }
        else
        {
            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        }
        //texture_handle = reinterpret_cast<Rml::Core::TextureHandle>(texture.get());
        osg::StateSet* ss = new osg::StateSet();
        ss->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
        ss->ref();
        texture_handle = reinterpret_cast<Rml::Core::TextureHandle>(ss);
    }

    /// Called by RmlUI when a texture is required by the library.
    /// @param[out] texture_handle The handle to write the texture handle for the loaded texture to.
    /// @param[out] texture_dimensions The variable to write the dimensions of the loaded texture.
    /// @param[in] source The application-defined image source, joined with the path of the referencing document.
    /// @return True if the load attempt succeeded and the handle and dimensions are valid, false if not.
    bool RenderInterface::LoadTexture(Rml::Core::TextureHandle& texture_handle, Rml::Core::Vector2i& texture_dimensions, const Rml::Core::String& source)
    {

        std::string src = source;

      // Hack: Sometimes RmlUI attaches that string to paths, sometimes it doesn't (when cloning elements)
      /*if(src.substr(0, 10) != "RmlUI/")
      {
         src = "RmlUI/" + src;
      }*/
      std::string path = osgDB::findDataFile(src);
      if(path.empty())
      {
         return false;
      }
      osg::ref_ptr<osg::Image> img = osgDB::readImageFile(path);
      if(!img.valid())
      {
         return false;
      }
      img->flipVertical();

      if(img == NULL) return false;

        texture_dimensions.x = img->s();
        texture_dimensions.y = img->t();

        AddTexture(texture_handle, img.get());

        return true;
    }

    /// Called by RmlUI when a texture is required to be built from an internally-generated sequence of pixels.
    /// @param[out] texture_handle The handle to write the texture handle for the generated texture to.
    /// @param[in] source The raw 8-bit texture data. Each pixel is made up of four 8-bit values, indicating red, green, blue and alpha in that order.
    /// @param[in] source_dimensions The dimensions, in pixels, of the source data.
    /// @return True if the texture generation succeeded and the handle is valid, false if not.
    bool RenderInterface::GenerateTexture(Rml::Core::TextureHandle& texture_handle, const Rml::Core::byte* source, const Rml::Core::Vector2i& source_dimensions)
    {

        osg::ref_ptr<osg::Image> img = new osg::Image();
        int w = source_dimensions.x;
        int h = source_dimensions.y;
        img->allocateImage(w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        memcpy(img->data(), source, w * h * 4 * sizeof(Rml::Core::byte));

        AddTexture(texture_handle, img.get());
        return true;
    }



    /// Called by RmlUI when a loaded texture is no longer required.
    /// @param texture The texture handle to release.
    void RenderInterface::ReleaseTexture(Rml::Core::TextureHandle th)
    {
        osg::Texture2D* texture = reinterpret_cast<osg::Texture2D*>(th);
        texture->unref();
    }

    /// Called when this render interface is released.
    void RenderInterface::Release()
    {

    }

}
#endif
