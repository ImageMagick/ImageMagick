//  
//  POV script for creating a spinning DNA molecule image sequence.
// 

#declare atom_O =
  sphere
  {
    <0.0 0.0 0.0> 2.22429
    texture
    {
      pigment { color red 0.90 green 0.00 blue 0.00 }
      finish
      {
        ambient 0.3
        diffuse 0.70
        specular 0.2
        reflection 0.00
        phong 0.8
        phong_size 100
      }
    }
  }
#declare atom_C =
  sphere
  {
    <0.0 0.0 0.0> 2.70093
    texture
    {
      pigment { color red 0.32 green 0.32 blue 0.32 }
      finish
      {
        ambient 0.3
        diffuse 0.70
        specular 0.2
        reflection 0.00
        phong 0.8
        phong_size 100
      }
    }
  }
#declare atom_N =
  sphere
  {
    <0.0 0.0 0.0> 2.44672
    texture
    {
      pigment { color red 0.00 green 0.00 blue 0.90 }
      finish
      {
        ambient 0.3
        diffuse 0.70
        specular 0.2
        reflection 0.00
        phong 0.8
        phong_size 100
      }
    }
  }
#declare atom_P =
  sphere
  {
    <0.0 0.0 0.0> 2.85981
    texture
    {
      pigment { color red 0.90 green 0.90 blue 0.00 }
      finish
      {
        ambient 0.3
        diffuse 0.70
        specular 0.2
        reflection 0.00
        phong 0.8
        phong_size 100
      }
    }
  }

#declare molecule_dna =
  union
  {
    object {atom_O translate <40.71169, 7.07008, -19.96654>}
    object {atom_C translate <37.85082, 6.08080, -20.30633>}
    object {atom_C translate <37.49451, 3.45211, -18.52562>}
    object {atom_O translate <38.32873, 3.99314, -15.61836>}
    object {atom_C translate <34.51841, 2.21794, -18.35552>}
    object {atom_O translate <34.71203, -0.70266, -17.64078>}
    object {atom_C translate <33.52023, 3.89230, -15.79419>}
    object {atom_C translate <35.99492, 3.13117, -13.87875>}
    object {atom_N translate <36.03749, 4.88010, -11.25832>}
    object {atom_C translate <36.92636, 3.82070, -8.70081>}
    object {atom_O translate <37.67140, 1.31954, -8.53515>}
    object {atom_N translate <36.90455, 5.49210, -6.42207>}
    object {atom_C translate <36.01080, 8.17164, -6.60404>}
    object {atom_N translate <35.88624, 9.80427, -4.23124>}
    object {atom_C translate <35.07702, 9.31980, -9.15562>}
    object {atom_C translate <35.11621, 7.59035, -11.53816>}
    object {atom_P translate <32.73897, -3.13710, -18.92938>}
    object {atom_O translate <29.80989, -2.23849, -19.56130>}
    object {atom_O translate <34.33516, -4.55303, -21.20071>}
    object {atom_O translate <32.36465, -5.16714, -16.25599>}
    object {atom_C translate <34.68449, -6.67585, -15.05636>}
    object {atom_C translate <33.44990, -7.80960, -12.35628>}
    object {atom_O translate <33.48803, -5.51646, -10.36331>}
    object {atom_C translate <30.39753, -8.91307, -12.52532>}
    object {atom_O translate <30.15646, -11.74682, -11.38013>}
    object {atom_C translate <28.79095, -6.59705, -10.89502>}
    object {atom_C translate <30.95784, -5.60459, -8.70737>}
    object {atom_N translate <30.48375, -2.56366, -7.96425>}
    object {atom_C translate <29.88446, -0.41012, -9.80639>}
    object {atom_N translate <29.66034, 2.12028, -8.53303>}
    object {atom_C translate <30.14777, 1.53858, -5.78062>}
    object {atom_C translate <30.16726, 3.41715, -3.42393>}
    object {atom_O translate <29.73977, 5.98547, -3.38453>}
    object {atom_N translate <30.81315, 2.01606, -0.96259>}
    object {atom_C translate <31.39126, -0.79990, -0.67407>}
    object {atom_N translate <31.97508, -1.77965, 1.97941>}
    object {atom_N translate <31.35694, -2.55942, -2.88353>}
    object {atom_C translate <30.71889, -1.25853, -5.35568>}
    object {atom_P translate <27.18967, -13.35212, -11.00221>}
    object {atom_O translate <24.95945, -12.07411, -12.77593>}
    object {atom_O translate <27.65550, -16.41021, -11.43097>}
    object {atom_O translate <26.44294, -12.83249, -7.74902>}
    object {atom_C translate <28.26284, -13.92642, -5.58891>}
    object {atom_C translate <27.19094, -12.70136, -2.86786>}
    object {atom_O translate <27.28097, -9.69644, -3.05915>}
    object {atom_C translate <24.15446, -13.41398, -2.19782>}
    object {atom_O translate <24.03245, -15.73678, -0.25823>}
    object {atom_C translate <22.93004, -10.68657, -0.99606>}
    object {atom_C translate <25.39159, -8.62645, -0.97000>}
    object {atom_N translate <24.47540, -5.90836, -2.20523>}
    object {atom_C translate <24.37583, -3.55252, -0.56794>}
    object {atom_O translate <25.00690, -3.78935, 1.95505>}
    object {atom_N translate <23.63546, -1.07084, -1.72330>}
    object {atom_C translate <22.98385, -0.89290, -4.46935>}
    object {atom_N translate <22.33986, 1.67034, -5.64907>}
    object {atom_C translate <23.04486, -3.27183, -6.20960>}
    object {atom_C translate <23.81722, -5.84799, -5.00678>}
    object {atom_P translate <21.07498, -17.05505, 0.69737>}
    object {atom_O translate <19.00914, -16.62078, -1.59916>}
    object {atom_O translate <21.56136, -19.97883, 1.65593>}
    object {atom_O translate <20.24712, -15.15063, 3.33009>}
    object {atom_C translate <21.99478, -15.13029, 5.81049>}
    object {atom_C translate <20.63542, -13.14177, 7.88163>}
    object {atom_O translate <20.53564, -10.34467, 6.68305>}
    object {atom_C translate <17.59046, -13.83490, 8.68555>}
    object {atom_O translate <16.87848, -12.69585, 11.35873>}
    object {atom_C translate <16.01778, -12.28213, 6.34665>}
    object {atom_C translate <17.63071, -9.50409, 6.55298>}
    object {atom_N translate <17.27461, -7.72805, 3.98975>}
    object {atom_C translate <16.68422, -8.53303, 1.26467>}
    object {atom_N translate <16.49314, -6.32441, -0.51244>}
    object {atom_C translate <16.97529, -4.07067, 1.17570>}
    object {atom_C translate <16.99393, -1.13354, 0.49422>}
    object {atom_O translate <16.58699, 0.01229, -1.80549>}
    object {atom_N translate <17.49090, 0.48744, 2.85070>}
    object {atom_C translate <17.95037, -0.45587, 5.54718>}
    object {atom_N translate <18.32702, 1.52121, 7.61366>}
    object {atom_N translate <17.95016, -3.20531, 6.18481>}
    object {atom_C translate <17.44875, -4.85278, 3.91900>}
    object {atom_P translate <14.40654, -14.08572, 13.19219>}
    object {atom_O translate <12.08692, -15.04852, 11.28078>}
    object {atom_O translate <15.64346, -16.22316, 15.08623>}
    object {atom_O translate <13.38739, -11.39665, 14.97056>}
    object {atom_C translate <15.46552, -9.51574, 16.17613>}
    object {atom_C translate <14.23962, -6.60806, 15.89333>}
    object {atom_O translate <14.04176, -5.99458, 12.94285>}
    object {atom_C translate <11.24889, -6.21298, 17.06288>}
    object {atom_O translate <11.17284, -4.12533, 19.27532>}
    object {atom_C translate <9.47899, -5.52515, 14.41470>}
    object {atom_C translate <11.68973, -4.12130, 12.55964>}
    object {atom_N translate <10.96842, -4.45219, 9.52973>}
    object {atom_C translate <10.62673, -6.91459, 8.03436>}
    object {atom_N translate <10.25093, -6.48032, 5.24531>}
    object {atom_C translate <10.32740, -3.63534, 4.99598>}
    object {atom_C translate <10.07828, -1.81905, 2.59628>}
    object {atom_N translate <9.69379, -2.83079, -0.08007>}
    object {atom_N translate <10.23292, 0.96280, 3.05300>}
    object {atom_C translate <10.63626, 1.93514, 5.67767>}
    object {atom_N translate <10.91313, 0.48871, 8.07864>}
    object {atom_C translate <10.72671, -2.33170, 7.56048>}
    object {atom_P translate <8.31198, -3.29196, 20.86833>}
    object {atom_O translate <6.27092, -5.65183, 20.68340>}
    object {atom_O translate <9.05468, -2.20142, 23.70442>}
    object {atom_O translate <7.11319, -0.67280, 19.11326>}
    object {atom_C translate <8.46386, 2.03449, 19.34734>}
    object {atom_C translate <6.80708, 3.92239, 17.43551>}
    object {atom_O translate <6.64948, 2.42512, 14.80935>}
    object {atom_C translate <3.74942, 4.50790, 18.29154>}
    object {atom_O translate <3.21834, 7.48825, 18.68979>}
    object {atom_C translate <2.07442, 3.06804, 15.92807>}
    object {atom_C translate <4.11929, 3.35636, 13.47880>}
    object {atom_N translate <3.58122, 1.22718, 11.22993>}
    object {atom_C translate <3.44924, -1.65403, 11.49982>}
    object {atom_N translate <3.14759, -2.98627, 9.00861>}
    object {atom_C translate <3.10204, -0.85985, 7.09783>}
    object {atom_C translate <2.93406, -0.87277, 4.07067>}
    object {atom_N translate <2.81606, -3.31441, 2.52680>}
    object {atom_N translate <3.00714, 1.62289, 2.75770>}
    object {atom_C translate <3.20966, 3.99335, 4.28526>}
    object {atom_N translate <3.38675, 4.28759, 7.08385>}
    object {atom_C translate <3.32108, 1.73071, 8.36653>}
    object {atom_P translate <0.10285, 8.68725, 19.28421>}
    object {atom_O translate <-1.69587, 6.55595, 20.74822>}
    object {atom_O translate <0.29880, 11.49346, 20.66306>}
    object {atom_O translate <-1.00951, 9.18168, 16.15198>}
    object {atom_C translate <0.56804, 10.97234, 14.29226>}
    object {atom_C translate <-1.14106, 10.93951, 11.62650>}
    object {atom_O translate <-1.18576, 8.11889, 10.59294>}
    object {atom_C translate <-4.23792, 11.67035, 12.09402>}
    object {atom_O translate <-4.76306, 14.57527, 11.41508>}
    object {atom_C translate <-5.84132, 9.46702, 10.31819>}
    object {atom_C translate <-3.45581, 8.05449, 8.64679>}
    object {atom_N translate <-3.87737, 5.00297, 8.04369>}
    object {atom_C translate <-3.90893, 4.12427, 5.31352>}
    object {atom_O translate <-3.72337, 5.86854, 3.37203>}
    object {atom_N translate <-4.09281, 1.28395, 4.78774>}
    object {atom_C translate <-4.29554, -0.69673, 6.89743>}
    object {atom_O translate <-4.34892, -3.22205, 6.21828>}
    object {atom_C translate <-4.37074, 0.25018, 9.69644>}
    object {atom_C translate <-4.70417, -1.80803, 12.10652>}
    object {atom_C translate <-4.12056, 3.14431, 10.23620>}
    object {atom_P translate <-7.89085, 15.85287, 11.39983>}
    object {atom_O translate <-9.72324, 14.49224, 13.55316>}
    object {atom_O translate <-7.60254, 18.95671, 11.65319>}
    object {atom_O translate <-8.95745, 15.10741, 8.26315>}
    object {atom_C translate <-7.40150, 16.18715, 5.85244>}
    object {atom_C translate <-8.71765, 14.83838, 3.29831>}
    object {atom_O translate <-8.79370, 11.86863, 3.87853>}
    object {atom_C translate <-11.80158, 15.64463, 2.76046>}
    object {atom_O translate <-12.16064, 17.46813, 0.32030>}
    object {atom_C translate <-13.35668, 12.77995, 2.71915>}
    object {atom_C translate <-10.93262, 10.75500, 2.07516>}
    object {atom_N translate <-11.38384, 7.79605, 3.06720>}
    object {atom_C translate <-11.18132, 5.60056, 1.20154>}
    object {atom_O translate <-10.81908, 6.04013, -1.33648>}
    object {atom_N translate <-11.24996, 2.90154, 2.22196>}
    object {atom_C translate <-11.58995, 2.32153, 5.04025>}
    object {atom_O translate <-11.53488, -0.18493, 5.76834>}
    object {atom_C translate <-11.91936, 4.58141, 6.92688>}
    object {atom_C translate <-12.36613, 4.07491, 10.05064>}
    object {atom_C translate <-11.73273, 7.34632, 5.90116>}
    object {atom_P translate <-15.22212, 18.21168, -0.93759>}
    object {atom_O translate <-17.41443, 18.11148, 1.28818>}
    object {atom_O translate <-15.04037, 20.86283, -2.55879>}
    object {atom_O translate <-15.62313, 15.69229, -3.13922>}
    object {atom_C translate <-13.58462, 15.32857, -5.38322>}
    object {atom_C translate <-14.53767, 12.82126, -7.11054>}
    object {atom_O translate <-14.70058, 10.42008, -5.26183>}
    object {atom_C translate <-17.43349, 13.06021, -8.49893>}
    object {atom_O translate <-17.30872, 12.41135, -11.46274>}
    object {atom_C translate <-19.26123, 11.06004, -6.71123>}
    object {atom_C translate <-17.10917, 8.76690, -6.07995>}
    object {atom_N translate <-17.84065, 7.03830, -3.52921>}
    object {atom_C translate <-17.77837, 4.14757, -3.70737>}
    object {atom_O translate <-17.39896, 3.04580, -6.03547>}
    object {atom_N translate <-18.08934, 2.57171, -1.37038>}
    object {atom_C translate <-18.52488, 3.80079, 1.13248>}
    object {atom_N translate <-18.80832, 2.16223, 3.48240>}
    object {atom_C translate <-18.71342, 6.74258, 1.39622>}
    object {atom_C translate <-18.34058, 8.38877, -1.02085>}
    object {atom_P translate <-19.67389, 13.58197, -13.60464>}
    object {atom_O translate <-21.14785, 16.07657, -12.37047>}
    object {atom_O translate <-18.27830, 13.88998, -16.41424>}
    object {atom_O translate <-21.90920, 11.05030, -13.75610>}
    object {atom_C translate <-21.09341, 8.41631, -15.05869>}
    object {atom_C translate <-23.35880, 6.29836, -14.39437>}
    object {atom_O translate <-23.15946, 5.68000, -11.42927>}
    object {atom_C translate <-26.43257, 7.23214, -14.84410>}
    object {atom_O translate <-28.10651, 4.95912, -15.90731>}
    object {atom_C translate <-27.31466, 8.06720, -11.86079>}
    object {atom_C translate <-26.00402, 5.53108, -10.44698>}
    object {atom_N translate <-25.97373, 5.66899, -7.33000>}
    object {atom_C translate <-26.11396, 7.92569, -5.52472>}
    object {atom_N translate <-26.05380, 7.14020, -2.79435>}
    object {atom_C translate <-25.85595, 4.29289, -2.92294>}
    object {atom_C translate <-25.68944, 2.25544, -0.69864>}
    object {atom_O translate <-25.67440, 2.63844, 1.87519>}
    object {atom_N translate <-25.50154, -0.43660, -1.76101>}
    object {atom_C translate <-25.47464, -1.20938, -4.54370>}
    object {atom_N translate <-25.26407, -4.01093, -5.17392>}
    object {atom_N translate <-25.61191, 0.68402, -6.61569>}
    object {atom_C translate <-25.77990, 3.34979, -5.64102>}
    object {atom_P translate <-28.94263, 4.89154, -19.18211>}
    object {atom_O translate <-31.69737, 6.36064, -19.42847>}
    object {atom_O translate <-26.58594, 5.98590, -20.93951>}
    object {atom_O translate <-29.17777, 1.57904, -19.78521>}
    object {atom_C translate <-26.58657, -0.06292, -19.97396>}
    object {atom_C translate <-27.00368, -2.86765, -18.48792>}
    object {atom_O translate <-27.59047, -2.24378, -15.58553>}
    object {atom_C translate <-29.44532, -4.74304, -19.46004>}
    object {atom_O translate <-28.63949, -7.67318, -19.74305>}
    object {atom_C translate <-31.71431, -4.16727, -17.22388>}
    object {atom_C translate <-29.82981, -4.05097, -14.63311>}
    object {atom_N translate <-31.15294, -2.47511, -12.26519>}
    object {atom_C translate <-31.40058, -3.74571, -9.69729>}
    object {atom_O translate <-30.63331, -6.22273, -9.49414>}
    object {atom_N translate <-32.39304, -2.28086, -7.48168>}
    object {atom_C translate <-33.15524, 0.42855, -7.77825>}
    object {atom_N translate <-34.03838, 1.95166, -5.48702>}
    object {atom_C translate <-32.98386, 1.78050, -10.38491>}
    object {atom_C translate <-31.93886, 0.25654, -12.67869>}
    object {atom_P translate <-30.57823, -9.74750, -21.58138>}
    object {atom_O translate <-32.10706, -8.03818, -23.70442>}
    object {atom_O translate <-28.82951, -12.12135, -22.65328>}
    object {atom_O translate <-32.82625, -10.98759, -19.37043>}
    object {atom_C translate <-32.08164, -13.32416, -17.54015>}
    object {atom_C translate <-34.59107, -13.75525, -15.62557>}
    object {atom_O translate <-34.97344, -11.13779, -14.18867>}
    object {atom_C translate <-37.50214, -14.33738, -16.99891>}
    object {atom_O translate <-38.51388, -17.17622, -16.55998>}
    object {atom_C translate <-39.41101, -12.06097, -15.67344>}
    object {atom_C translate <-37.74406, -11.30514, -13.02928>}
    object {atom_N translate <-38.39165, -8.41800, -12.04403>}
    object {atom_C translate <-38.78164, -6.02847, -13.64425>}
    object {atom_N translate <-39.31568, -3.73576, -12.05970>}
    object {atom_C translate <-39.23582, -4.71847, -9.37699>}
    object {atom_C translate <-39.57624, -3.22756, -6.75889>}
    object {atom_O translate <-40.03593, -0.69334, -6.35407>}
    object {atom_N translate <-39.40232, -5.03030, -4.48503>}
    object {atom_C translate <-38.94434, -7.87294, -4.55430>}
    object {atom_N translate <-38.95958, -9.27107, -2.03957>}
    object {atom_N translate <-38.53654, -9.25794, -6.98598>}
    object {atom_C translate <-38.69881, -7.55392, -9.28590>}
    object {atom_O translate <-43.67996, 2.27662, 12.88651>}
    object {atom_C translate <-45.27319, -0.20591, 13.69297>}
    object {atom_C translate <-43.85134, -2.82549, 12.52342>}
    object {atom_O translate <-43.94963, -2.75410, 9.47931>}
    object {atom_C translate <-40.77376, -3.28984, 13.31166>}
    object {atom_O translate <-40.23019, -6.16490, 14.04801>}
    object {atom_C translate <-39.22968, -2.33106, 10.64526>}
    object {atom_C translate <-41.20358, -3.45910, 8.38178>}
    object {atom_N translate <-41.00255, -1.74109, 5.76135>}
    object {atom_C translate <-40.31704, -2.94391, 3.22459>}
    object {atom_O translate <-39.78299, -5.47982, 3.15702>}
    object {atom_N translate <-40.33695, -1.38796, 0.86387>}
    object {atom_C translate <-41.04830, 1.33182, 0.94628>}
    object {atom_N translate <-41.18367, 2.82613, -1.51612>}
    object {atom_C translate <-41.75711, 2.65835, 3.48897>}
    object {atom_C translate <-41.73804, 1.03695, 5.94861>}
    object {atom_P translate <-37.51845, -6.92349, 15.94247>}
    object {atom_O translate <-35.70618, -4.38123, 16.17084>}
    object {atom_O translate <-38.39589, -8.24366, 18.66776>}
    object {atom_O translate <-35.97670, -9.20286, 13.97938>}
    object {atom_C translate <-37.52226, -11.58222, 12.87824>}
    object {atom_C translate <-35.67229, -12.66111, 10.56159>}
    object {atom_O translate <-35.61192, -10.58531, 8.37055>}
    object {atom_C translate <-32.61124, -13.03755, 11.46084>}
    object {atom_O translate <-31.72236, -15.89142, 11.06280>}
    object {atom_C translate <-30.99343, -10.85371, 9.66064>}
    object {atom_C translate <-32.87540, -10.64590, 7.07305>}
    object {atom_N translate <-32.65042, -7.79499, 5.81770>}
    object {atom_C translate <-32.75656, -5.27327, 7.23701>}
    object {atom_N translate <-32.64238, -3.03161, 5.50015>}
    object {atom_C translate <-32.44833, -4.17977, 2.89751>}
    object {atom_C translate <-32.28628, -2.78291, 0.21671>}
    object {atom_O translate <-32.33415, -0.22285, -0.29551>}
    object {atom_N translate <-32.03885, -4.69114, -1.96098>}
    object {atom_C translate <-31.99373, -7.56811, -1.73029>}
    object {atom_N translate <-31.63805, -9.09461, -4.14820>}
    object {atom_N translate <-32.19328, -8.87176, 0.78062>}
    object {atom_C translate <-32.43139, -7.06923, 2.99665>}
    object {atom_P translate <-28.56810, -16.87456, 11.72267>}
    object {atom_O translate <-27.31085, -14.90235, 13.79317>}
    object {atom_O translate <-28.69542, -19.90659, 12.48338>}
    object {atom_O translate <-27.03863, -16.55130, 8.71140>}
    object {atom_C translate <-27.92687, -18.21803, 6.31318>}
    object {atom_C translate <-26.44697, -16.92540, 3.81731>}
    object {atom_O translate <-27.05156, -13.97810, 3.83426>}
    object {atom_C translate <-23.24526, -16.98662, 4.01919>}
    object {atom_O translate <-22.15747, -19.60705, 2.99793>}
    object {atom_C translate <-22.38986, -14.46745, 2.23722>}
    object {atom_C translate <-25.08253, -12.74627, 1.95632>}
    object {atom_N translate <-24.77451, -9.81740, 2.97971>}
    object {atom_C translate <-24.86179, -7.68462, 1.05601>}
    object {atom_O translate <-25.00288, -8.32818, -1.46232>}
    object {atom_N translate <-24.81159, -4.99492, 1.92391>}
    object {atom_C translate <-24.63788, -4.38589, 4.67653>}
    object {atom_N translate <-24.64678, -1.63708, 5.55396>}
    object {atom_C translate <-24.48324, -6.51803, 6.70699>}
    object {atom_C translate <-24.58259, -9.31513, 5.80456>}
    object {atom_P translate <-18.85514, -20.22117, 2.69775>}
    object {atom_O translate <-17.23987, -18.76966, 4.94471>}
    object {atom_O translate <-18.57911, -23.30425, 2.53887>}
    object {atom_O translate <-18.08595, -18.96921, -0.33237>}
    object {atom_C translate <-19.12290, -20.31756, -2.84837>}
    object {atom_C translate <-18.37935, -18.30807, -5.17901>}
    object {atom_O translate <-19.10680, -15.57155, -4.06834>}
    object {atom_C translate <-15.28483, -18.12334, -6.02890>}
    object {atom_O translate <-14.94123, -18.09856, -9.04907>}
    object {atom_C translate <-14.22987, -15.49211, -4.45389>}
    object {atom_C translate <-16.82976, -13.61777, -4.58713>}
    object {atom_N translate <-16.80412, -11.59048, -2.17155>}
    object {atom_C translate <-16.19403, -12.09847, 0.62132>}
    object {atom_N translate <-16.40460, -9.77546, 2.25310>}
    object {atom_C translate <-17.12591, -7.72021, 0.40715>}
    object {atom_C translate <-17.62966, -4.77779, 0.89078>}
    object {atom_O translate <-17.51060, -3.43664, 3.10935>}
    object {atom_N translate <-18.20946, -3.37606, -1.58730>}
    object {atom_C translate <-18.35224, -4.53057, -4.21620>}
    object {atom_N translate <-18.80853, -2.75791, -6.44092>}
    object {atom_N translate <-17.95292, -7.28531, -4.66170>}
    object {atom_C translate <-17.35744, -8.73915, -2.28530>}
    object {atom_P translate <-11.90707, -17.62044, -10.43893>}
    object {atom_O translate <-9.71053, -18.33052, -8.31060>}
    object {atom_O translate <-11.68507, -19.09970, -13.18541>}
    object {atom_O translate <-12.06235, -14.34246, -11.16935>}
    object {atom_C translate <-14.36058, -13.41059, -12.94540>}
    object {atom_C translate <-13.91318, -10.41330, -13.86689>}
    object {atom_O translate <-14.06040, -8.57349, -11.45025>}
    object {atom_C translate <-11.09404, -9.82482, -15.27900>}
    object {atom_O translate <-11.32791, -7.70305, -17.41559>}
    object {atom_C translate <-9.31969, -8.95755, -12.71704>}
    object {atom_C translate <-11.41413, -7.12240, -11.11872>}
    object {atom_N translate <-10.86314, -7.14338, -7.99984>}
    object {atom_C translate <-10.17424, -9.37106, -6.26001>}
    object {atom_N translate <-9.99249, -8.61988, -3.51354>}
    object {atom_C translate <-10.58923, -5.82426, -3.53070>}
    object {atom_C translate <-10.79069, -3.74550, -1.32399>}
    object {atom_N translate <-10.42167, -4.36025, 1.47714>}
    object {atom_N translate <-11.38786, -1.08927, -2.09423>}
    object {atom_C translate <-11.81599, -0.48045, -4.82608>}
    object {atom_N translate <-11.73761, -2.20417, -7.06351>}
    object {atom_C translate <-11.09976, -4.86527, -6.22654>}
    object {atom_P translate <-8.58800, -6.64619, -19.06411>}
    object {atom_O translate <-6.35057, -8.85396, -18.84359>}
    object {atom_O translate <-9.36608, -5.77321, -21.97646>}
    object {atom_O translate <-7.84763, -3.85714, -17.32196>}
    object {atom_C translate <-9.95457, -1.68496, -17.07983>}
    object {atom_C translate <-8.60580, 0.80308, -15.64251>}
    object {atom_O translate <-8.27279, 0.23599, -12.67636>}
    object {atom_C translate <-5.72947, 1.72415, -16.78855>}
    object {atom_O translate <-5.69515, 4.70682, -17.31603>}
    object {atom_C translate <-3.75175, 0.72639, -14.42762>}
    object {atom_C translate <-5.55746, 1.23904, -11.82350>}
    object {atom_N translate <-4.77239, -0.67195, -9.45558>}
    object {atom_C translate <-4.18920, -3.51523, -9.53947>}
    object {atom_N translate <-3.71701, -4.61933, -6.95865>}
    object {atom_C translate <-4.03117, -2.38804, -5.20125>}
    object {atom_C translate <-3.84136, -2.14676, -2.19379>}
    object {atom_N translate <-3.28613, -4.39584, -0.48024>}
    object {atom_N translate <-4.26059, 0.40673, -1.06681>}
    object {atom_C translate <-4.85797, 2.59649, -2.73821>}
    object {atom_N translate <-5.12022, 2.65221, -5.54082>}
    object {atom_C translate <-4.67515, 0.04216, -6.63433>}
    object {atom_P translate <-2.89106, 6.35068, -18.26422>}
    object {atom_O translate <-0.73285, 4.34987, -19.31006>}
    object {atom_O translate <-3.69816, 8.70547, -20.15359>}
    object {atom_O translate <-1.92825, 7.77592, -15.36522>}
    object {atom_C translate <-3.84644, 9.66382, -13.93065>}
    object {atom_C translate <-2.45382, 10.35314, -11.15494>}
    object {atom_O translate <-1.95346, 7.74732, -9.70322>}
    object {atom_C translate <0.42865, 11.78410, -11.40195>}
    object {atom_O translate <0.29414, 14.70132, -10.55629>}
    object {atom_C translate <2.36336, 9.87714, -9.65047>}
    object {atom_C translate <0.25367, 8.35106, -7.76152>}
    object {atom_N translate <1.30206, 5.54082, -6.83430>}
    object {atom_C translate <1.64587, 5.00255, -4.01919>}
    object {atom_O translate <1.24762, 6.88070, -2.24188>}
    object {atom_N translate <2.37544, 2.32937, -3.20553>}
    object {atom_C translate <2.86245, 0.21184, -5.11228>}
    object {atom_O translate <3.50517, -2.13384, -4.18210>}
    object {atom_C translate <2.60719, 0.81960, -8.00005>}
    object {atom_C translate <3.19695, -1.43350, -10.16376>}
    object {atom_C translate <1.75624, 3.51989, -8.84591>}
    object {atom_P translate <3.03764, 16.69132, -10.55036>}
    object {atom_O translate <5.02595, 15.78105, -12.79203>}
    object {atom_O translate <2.07336, 19.65344, -10.65374>}
    object {atom_O translate <4.28833, 16.15537, -7.46961>}
    object {atom_C translate <2.50551, 16.77775, -5.08156>}
    object {atom_C translate <4.17754, 16.06449, -2.50286>}
    object {atom_O translate <4.78636, 13.11423, -2.53400>}
    object {atom_C translate <7.02951, 17.52999, -2.29145>}
    object {atom_O translate <7.17526, 19.12110, 0.27030>}
    object {atom_C translate <9.12903, 15.05890, -2.48104>}
    object {atom_C translate <7.38180, 12.76131, -1.06660>}
    object {atom_N translate <8.24970, 9.86718, -1.87540>}
    object {atom_C translate <8.62550, 7.85685, 0.15591>}
    object {atom_O translate <8.34269, 8.44046, 2.68970>}
    object {atom_N translate <9.24237, 5.16248, -0.68593>}
    object {atom_C translate <9.46564, 4.40453, -3.46715>}
    object {atom_O translate <10.01324, 1.91078, -4.01220>}
    object {atom_C translate <9.06209, 6.48922, -5.52515>}
    object {atom_C translate <9.24618, 5.81812, -8.62094>}
    object {atom_C translate <8.44543, 9.24311, -4.68246>}
    object {atom_P translate <9.94122, 20.84397, 1.13736>}
    object {atom_O translate <11.60499, 21.50850, -1.43266>}
    object {atom_O translate <9.08412, 23.30425, 2.88205>}
    object {atom_O translate <11.55521, 18.63853, 3.11274>}
    object {atom_C translate <10.15645, 17.60307, 5.61709>}
    object {atom_C translate <12.29855, 15.80796, 7.07898>}
    object {atom_O translate <12.63579, 13.30128, 5.43130>}
    object {atom_C translate <15.20475, 17.20842, 7.19167>}
    object {atom_O translate <15.97181, 18.05831, 10.02182>}
    object {atom_C translate <17.15980, 15.01632, 5.84672>}
    object {atom_C translate <15.41849, 12.30798, 5.96323>}
    object {atom_N translate <15.89598, 10.50503, 3.44448>}
    object {atom_C translate <16.19488, 7.67276, 3.79804>}
    object {atom_O translate <16.24318, 6.75338, 6.24434>}
    object {atom_N translate <16.36858, 5.97933, 1.52460>}
    object {atom_C translate <16.23830, 7.05991, -1.08461>}
    object {atom_N translate <16.33384, 5.30801, -3.37415>}
    object {atom_C translate <15.94321, 9.96378, -1.53392>}
    object {atom_C translate <15.74959, 11.71271, 0.82956>}
    object {atom_P translate <18.86933, 19.72717, 10.62196>}
    object {atom_O translate <19.73256, 21.44792, 8.12037>}
    object {atom_O translate <18.60538, 21.27824, 13.34704>}
    object {atom_O translate <21.09743, 17.21604, 10.99056>}
    object {atom_C translate <20.92966, 15.30167, 13.35700>}
    object {atom_C translate <23.31008, 13.20426, 12.99814>}
    object {atom_O translate <22.72985, 11.47630, 10.55163>}
    object {atom_C translate <26.25759, 14.44033, 12.44207>}
    object {atom_O translate <28.35711, 12.63908, 13.55824>}
    object {atom_C translate <26.34253, 14.49287, 9.22044>}
    object {atom_C translate <25.23484, 11.52778, 8.83405>}
    object {atom_N translate <24.50230, 11.01407, 5.85138>}
    object {atom_C translate <23.93225, 12.92527, 3.74550>}
    object {atom_N translate <23.44417, 11.66907, 1.24264>}
    object {atom_C translate <23.71469, 8.87727, 1.82032>}
    object {atom_C translate <23.46493, 6.48308, -0.00741>}
    object {atom_O translate <22.99529, 6.42143, -2.57616>}
    object {atom_N translate <23.89030, 3.99695, 1.44727>}
    object {atom_C translate <24.49383, 3.70101, 4.25433>}
    object {atom_N translate <24.92640, 1.03822, 5.27243>}
    object {atom_N translate <24.75587, 5.93442, 5.95963>}
    object {atom_C translate <24.32097, 8.41843, 4.60217>}
    object {atom_P translate <29.80798, 13.32564, 16.52121>}
    object {atom_O translate <31.72512, 15.78275, 16.08123>}
    object {atom_O translate <27.61187, 13.65823, 18.75080>}
    object {atom_O translate <31.59971, 10.49316, 17.04382>}
    object {atom_C translate <30.17489, 7.87252, 17.78461>}
    object {atom_C translate <31.55077, 5.48342, 16.16427>}
    object {atom_O translate <30.68457, 5.91324, 13.30213>}
    object {atom_C translate <34.79718, 5.50079, 16.11639>}
    object {atom_O translate <36.09193, 3.18900, 17.63336>}
    object {atom_C translate <35.51086, 5.66390, 12.97357>}
    object {atom_C translate <32.79490, 4.64729, 11.58307>}
    object {atom_N translate <32.46613, 5.96980, 8.74042>}
    object {atom_C translate <32.10282, 4.27552, 6.44410>}
    object {atom_O translate <32.08503, 1.70148, 6.82371>}
    object {atom_N translate <31.78549, 5.42431, 3.86413>}
    object {atom_C translate <31.80434, 8.23794, 3.53006>}
    object {atom_N translate <31.47112, 9.38970, 0.89925>}
    object {atom_C translate <32.20027, 10.04725, 5.83507>}
    object {atom_C translate <32.54259, 8.85375, 8.50464>}
    object {atom_P translate <39.47520, 3.11317, 17.99391>}
    object {atom_O translate <40.71233, 5.96196, 17.42089>}
    object {atom_O translate <40.21684, 1.82943, 20.75648>}
    object {atom_O translate <40.48883, 1.07719, 15.48279>}
    object {atom_C translate <39.71415, -1.87137, 15.44000>}
    object {atom_C translate <40.62822, -2.85875, 12.56896>}
    object {atom_O translate <38.89434, -1.41253, 10.56116>}
    object {atom_C translate <43.58188, -1.93302, 11.74512>}
    object {atom_O translate <45.74982, -3.60908, 13.02547>}
    object {atom_C translate <43.40224, -2.34822, 8.56057>}
    object {atom_C translate <40.37020, -1.41889, 7.91764>}
    object {atom_N translate <40.14651, 1.48689, 6.79808>}
    object {atom_C translate <40.41321, 4.03148, 8.16274>}
    object {atom_N translate <39.94526, 6.24582, 6.45512>}
    object {atom_C translate <39.36356, 5.02648, 3.93658>}
    object {atom_C translate <38.63674, 6.31340, 1.30365>}
    object {atom_O translate <38.31496, 8.83977, 0.75965>}
    object {atom_N translate <38.32343, 4.31492, -0.79121>}
    object {atom_C translate <38.59670, 1.44770, -0.51752>}
    object {atom_N translate <38.36983, -0.17837, -2.89116>}
    object {atom_N translate <39.16803, 0.25060, 1.96903>}
    object {atom_C translate <39.50167, 2.15502, 4.06982>}
  }

object {molecule_dna rotate <clock,0,0>}

plane
{
  z, -50.0
  texture
  {
    pigment
    {
      marble
      turbulence 1
      color_map
      {
        [0.0, 0.8  color red 0.9 green 0.9 blue 0.9
          color red 0.5 green 0.5 blue 0.5]
        [0.8, 1.01 color red 0.5 green 0.5 blue 0.5
          color red 0.2 green 0.2 blue 0.2]
      }
      scale 30
    }
    finish { ambient 0.3 diffuse 0.7 }
  }
  rotate 5*x
}

light_source
{
  <465.31511, -843.39145, 268.64980>
  color red 1.00 green 1.00 blue 1.00
}

background { color red 0.0 green 0.0 blue 0.0 }

camera
{
  location <0.00, -150.00, 0.0>
  direction <0.0, -1.25, 0.0>
  up <0.00, 0.00, 1.00>
  sky <0.00, 0.00, 1.00>
  right <-1.0, 0.00, 0.00>
  look_at <0.00, 0.00, -0.10>
}
