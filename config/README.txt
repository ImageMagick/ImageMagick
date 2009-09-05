This directory contains the ImageMagick configure files.  Currently this
document only gives basic instructions for adding another locale to the
messaging subsystem: For now the only locale supported is C.  To translate to
another locale, do the following.    Assume the new locale is klingon.  Type

    cp english.xml klingon.xml

Edit klingon.xml and translate all the messages.  Do not translate
the tags:

    <Message name="ThisIsATag">
      this is a message
    </Message>

Save the messages to the ImageMagick install directory (the directory
where you find the files colors.xml and delegates.xml).  Edit locale.xml
in the ImageMagick install directory and add the locale alias to the
file:

   <include locale="kl_gn.IQR-6668-1" file="klingon.xml"/>

To test the message translataions, type

    convert logo: /image.jpg

Assuming the root partition is not writable by you, you should get
the message:

    convert: Unable to open file (/image.jpg) [Permission denied].

except in the klingon language.

The last step is the most important.  E-mail klingon.xml to
magick-developers@imagemagick.org with a note that you are contributing your
message translation to the ImageMagick project and that it can be redistributed
under the ImageMagick copyright.
