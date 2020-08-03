/* osgRmlUi, an interface for OpenSceneGraph to use RmlUI
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

#include <osgRmlUi/FileInterface>

namespace osgRmlUi
{

    Rml::Core::FileHandle FileInterface::Open(const Rml::Core::String& path)
    {
        std::string abspath = osgDB::findDataFile(path.c_str());
        if(!osgDB::fileExists(abspath))
        {
            return 0;
        }

        FILE* fp = fopen(abspath.c_str(), "rb");
        return (Rml::Core::FileHandle) fp;
    }

    /// Closes a previously opened file.
    /// @param file The file handle previously opened through Open().
    void FileInterface::Close(Rml::Core::FileHandle file)
    {
        fclose((FILE*) file);
    }

    /// Reads data from a previously opened file.
    /// @param buffer The buffer to be read into.
    /// @param size The number of bytes to read into the buffer.
    /// @param file The handle of the file.
    /// @return The total number of bytes read into the buffer.
    size_t FileInterface::Read(void* buffer, size_t size, Rml::Core::FileHandle file)
    {
        return fread(buffer, 1, size, (FILE*) file);
    }

    /// Seeks to a point in a previously opened file.
    /// @param file The handle of the file to seek.
    /// @param offset The number of bytes to seek.
    /// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
    /// @return True if the operation completed successfully, false otherwise.
    bool FileInterface::Seek(Rml::Core::FileHandle file, long offset, int origin)
    {
        return fseek((FILE*) file, offset, origin) == 0;
    }

    /// Returns the current position of the file pointer.
    /// @param file The handle of the file to be queried.
    /// @return The number of bytes from the origin of the file.
    size_t FileInterface::Tell(Rml::Core::FileHandle file)
    {
        return ftell((FILE*) file);
    }

}
