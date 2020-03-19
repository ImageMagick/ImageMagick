<p>The convex hull is the is the smallest area convex polygon containing the set of points inside it. The convex hull may be visualized as the shape enclosed by a rubber band stretched around the set of points.</p>

<p>The convex-hull string format returns a list of x,y coordinates of the vertices of the convex-hull polygon containing all the non-black pixels within it.</p>

<ul><pre class="code">
magick [-fuzz <var>XXX</var>%] [-define convex-hull:background-color] -format "%[convex-hull]" info:
</pre></ul>

<p>where the string format is sensitive to an optional <code>-fuzz</code> value and to an optional background color specified by the define. The command above will find the background color automatically according to the fuzz value unless the background color is defined. The fuzz value defaults to 0 when not specified.</p>

<p>For example, consider the following white rectangle on a black background:</p>

<ul><img class="img-fluid d-block" src="<?php echo $_SESSION['RelativePath']?>/../image/convex-hull.png" alt="[Convex Hull]" width="250" height="250" name="convert-hull" /></ul>

<p>The convex hull set of points may be computed as:</p>

<ul><pre class="code">magick rect.png -format "%[convex-hull]" info:
190,121 193,122 281,154 377,189 378,190 377,193 345,281 310,377 309,378 306,377 218,345 122,310 121,309 122,306 154,218 189,122 190,121
</pre></ul>

<p>The convex hull may also be drawn as a polygon (filled or unfilled) using <code>-draw</code> as follows:</p>

<ul><pre class="code">magick rect.png -set option:hull "%[convex-hull]" -fill none -stroke red -strokewidth 1 -draw "polygon %[hull]" blocks_hull.png</pre></ul>

<p>Here is an example using a non-convex shaped image on a black background:</p>

<ul><img class="img-fluid d-block" src="<?php echo $_SESSION['RelativePath']?>/../image/convex-hull-blocks.png" alt="[Convex Hull Blocks]" width="128" height="128" name="convert-hull-blocks" /></ul>

<ul><pre class="code">magick blocks_black.png -set option:hull "%[convex-hull]" -fill none -stroke red -strokewidth 1 -draw "polygon %[hull]" blocks_hull.png</pre></ul>

<ul><img class="img-fluid d-block" src="<?php echo $_SESSION['RelativePath']?>/../image/convex-hull-blocks-closure.png" alt="[Convex Hull Blocks Closure]" width="128" height="128" name="convert-hull" /></ul>

<p>Finally, here is an example with a non-constant, non-black background using -fuzz to determine the background color:</p>

<ul><img class="img-fluid d-block" src="<?php echo $_SESSION['RelativePath']?>/../image/convex-hull-barn.png" alt="[Convex Hull Barn]" width="250" height="250" name="convert-hull-barn" /></ul>

<ul><pre class="code">magick barn_rot20.png -fuzz 30% -set option:hull "%[convex-hull]" -fill none -stroke red -strokewidth 1 -draw "polygon %[hull]" barn_rot20_hull.png</pre></ul>

<ul><img class="img-fluid d-block" src="<?php echo $_SESSION['RelativePath']?>/../image/convex-hull-barn-closure.png" alt="[Convex Hull Barn Closure]" width="250" height="250" name="convert-hull" /></ul>
