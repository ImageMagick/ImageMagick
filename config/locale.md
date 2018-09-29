Translation is fairly straight-forward.  Copy `english.xml` to _locale_.xml.  Change any messages from English to the target language.  As an example:

```
<message name="UnableToOpenBlob">
  unable to open image
</message>
```
becomes
```
<message name="UnableToOpenBlob">
  impossible d'ouvrir l'image
</message>
```
in French.  You then need to copy the _locale_.xml to the same location as `english.xml`, e.g. `/usr/local/ImageMagick-7/_locale_.xml`. Make sure your your _locale_.xml file is referenced in the `locale.xml` configuration file.

The final step is to contribute _locale_.xml to ImageMagick so we can include it in a future release of ImageMagick so the community can benefit from the translation.
