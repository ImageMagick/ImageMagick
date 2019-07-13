$!
$! Make ImageMagick image coders for VMS.
$!
$
$ define/nolog MAGICKCORE [-.magickcore]
$ define/nolog MAGICKWAND [-.magickwand]
$if (f$trnlnm("X11") .eqs. "") then define/nolog X11 decw$include:
$compile_options="/nodebug/optimize"
$if (f$search("sys$system:decc$compiler.exe") .nes. "") 
$then     ! VAX with DEC C
$  compile_options="/decc/nodebug/optimize/warning=(disable=rightshiftovr)"
$else     ! VAX with VAX C
$define/nolog lnk$library sys$library:vaxcrtl
$define/nolog sys sys$share
$endif
$if (f$getsyi("HW_MODEL") .gt. 1023)
$then     ! Alpha with DEC C
$  define/nolog sys decc$library_include
$  compile_options="/nodebug/optimize/prefix=all/warning=(disable=(rightshiftovr,INPTRTYPE))/name=(as_is,short)/float=ieee"
$endif
$
$
$write sys$output "Making Coders..."
$call Make aai.c
$call Make art.c
$call Make avs.c
$call Make bgr.c
$call Make bmp.c
$call Make braille.c
$call Make cals.c
$call Make caption.c
$call Make cip.c
$call Make clip.c
$call Make clipboard.c
$call Make cmyk.c
$call Make cut.c
$call Make dcm.c
$call Make dds.c
$call Make debug.c
$call Make dib.c
$call Make dng.c
$call Make dps.c
$call Make dpx.c
$call Make emf.c
$call Make ept.c
$call Make exr.c
$call Make fax.c
$call Make fd.c
$call Make fits.c
$call Make fpx.c
$call Make gif.c
$call Make gradient.c
$call Make gray.c
$call Make hald.c
$call Make hdr.c
$call Make histogram.c
$call Make hrz.c
$call Make html.c
$call Make icon.c
$call Make info.c
$call Make inline.c
$call Make ipl.c
$call Make jbig.c
$call Make jnx.c
$call Make jpeg.c
$call Make jp2.c
$call Make json.c
$call Make label.c
$call Make mac.c
$call Make magick.c
$call Make map.c
$call Make mat.c
$call Make mask.c
$call Make matte.c
$call Make meta.c
$call Make miff.c
$call Make mpc.c
$call Make mpeg.c
$call Make mpr.c
$call Make msl.c
$call Make mono.c
$call Make mtv.c
$call Make mvg.c
$call Make null.c
$call Make otb.c
$call Make palm.c
$call Make pango.c
$call Make pattern.c
$call Make pcd.c
$call Make pcl.c
$call Make pcx.c
$call Make pdb.c
$call Make pdf.c
$call Make pes.c
$call Make pict.c
$call Make pix.c
$call Make plasma.c
$call Make png.c
$call Make pnm.c
$call Make preview.c
$call Make ps.c
$call Make ps2.c
$call Make ps3.c
$call Make psd.c
$call Make pwp.c
$call Make raw.c
$call Make rgb.c
$call Make rgf.c
$call Make rla.c
$call Make rle.c
$call Make screenshot.c
$call Make sct.c
$call Make sfw.c
$call Make sgi.c
$call Make sixel.c
$call Make stegano.c
$call Make sun.c
$call Make svg.c
$call Make tga.c
$call Make thumbnail.c
$call Make tiff.c
$call Make tile.c
$call Make tim.c
$call Make tim2.c
$call Make ttf.c
$call Make txt.c
$call Make uil.c
$call Make url.c
$call Make uyvy.c
$call Make vicar.c
$call Make vid.c
$call Make viff.c
$call Make vips.c
$call Make webp.c
$call Make wbmp.c
$call Make wmf.c
$call Make wpg.c
$call Make x.c
$call Make xbm.c
$call Make xc.c
$call Make xcf.c
$call Make xpm.c
$call Make xps.c
$call Make xtrn.c
$call Make xwd.c
$call Make ycbcr.c
$call Make yuv.c
$call Make cin.c
$call Make magick.c
$call Make scr.c
$deass magickcore
$deass magickwand
$library/create libCoders.olb aai,art,avs,bgr,bmp,braille,clip,clipboard,cip, -
  cmyk,cut,dcm,dds,debug,dib,dng,dps,dpx,emf,ept,exr,fax,fd,fits,fpx,gif, -
  gradient,gray,hald,histogram,hrz,html,icon,info,inline,ipl,jbig,jpeg,jp2, -
  jnx,json,hdr,label,cals,caption,palm,mac,magick,map,mat,matte,pango,rgf, -
  meta,miff,mpc,mpr,msl,mpeg,mono,mtv,mvg,null,otb,pattern,pcd,pcl,pcx,pdb, -
  pdf,pes,pict,pix,plasma,png,pnm,preview,ps,ps2,ps3,psd,pwp,raw,rgb,rla,rle, -
  sct,sfw,sgi,stegano,sun,svg,tga,thumbnail,tiff,tile,tim,tim2,ttf,txt,uil,url, -
  uyvy,vicar,vid,viff,wbmp,webp,wmf,wpg,x,xbm,xc,xcf,xpm,xps,xwd,ycbcr,yuv, -
  mask,screenshot,vips,sixel,xtrn, -
  cin,magick,scr,[-.magickcore]compress,[-.magickcore]prervicccm
$exit
$
$Make: subroutine
$!
$! Primitive MMS hack for DCL.
$!
$if (p1 .eqs. "") then exit
$source_file=f$search(f$parse(p1,".c"))
$if (source_file .nes. "")
$then
$  object_file=f$parse(source_file,,,"name")+".obj"
$  object_file=f$search( object_file )
$  if (object_file .nes. "")
$  then
$    object_time=f$file_attribute(object_file,"cdt")
$    source_time=f$file_attribute(source_file,"cdt")
$    if (f$cvtime(object_time) .lts. f$cvtime(source_time)) then -
$      object_file=""
$  endif
$  if (object_file .eqs. "")
$  then
$    write sys$output "Compiling ",p1
$    cc'compile_options'/include_directory=([-],[-.magickcore],[-.jpeg],[-.png], -
       [-.tiff],[-.ttf],[-.zlib]) 'source_file'  
$  endif
$endif
$exit
$endsubroutine
