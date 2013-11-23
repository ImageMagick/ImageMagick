// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Reference counted container class for Binary Large Objects (BLOBs)
//

#if !defined(Magick_BlobRef_header)
#define Magick_BlobRef_header

#include "Magick++/Include.h"
#include <string>

namespace Magick
{
  // Forward decl
  class BlobRef;

  class MagickPPExport Blob
  {
  public:

    enum Allocator
    {
      MallocAllocator,
      NewAllocator
    };

    // Default constructor
    Blob(void);

    // Construct object with data, making a copy of the supplied data.
    Blob(const void* data_,size_t length_);

    // Copy constructor (reference counted)
    Blob(const Blob& blob_);

    // Destructor (reference counted)
    virtual ~Blob();

    // Assignment operator (reference counted)
    Blob& operator=(const Blob& blob_ );

    // Update object contents from Base64-encoded string representation.
    void base64(const std::string base64_);
    // Return Base64-encoded string representation.
    std::string base64(void);

    // Obtain pointer to data. The user should never try to modify or
    // free this data since the Blob class manages its own data. The
    // user must be finished with the data before allowing the Blob to
    // be destroyed since the pointer is invalid once the Blob is
    // destroyed.
    const void* data(void) const;

    // Obtain data length
    size_t length(void) const;

    // Update object contents, making a copy of the supplied data.
    // Any existing data in the object is deallocated.
    void update(const void* data_,size_t length_);

    // Update object contents, using supplied pointer directly (no
    // copy). Any existing data in the object is deallocated.  The user
    // must ensure that the pointer supplied is not deleted or
    // otherwise modified after it has been supplied to this method.
    // Specify allocator_ as "MallocAllocator" if memory is allocated
    // via the C language malloc() function, or "NewAllocator" if
    // memory is allocated via C++ 'new'.
    void updateNoCopy(void* data_,size_t length_,
      Allocator allocator_=NewAllocator);

  private:
    BlobRef *_blobRef;
  };

} // namespace Magick

#endif // Magick_BlobRef_header
