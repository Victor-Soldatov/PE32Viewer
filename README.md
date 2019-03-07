# PE32Viewer
32-bit Windows shell extension for view PE32 files structure and dependencies

Simple shell extension for PE32 images only. 
Not deep tested.
Shown info:
<ul>
<li>PE32 headers (DOS Stub, NT headers, optional headers, data directory, section descriptors)</li>
<li>Sections details</li>
<li>Import (static/delayed)</li>
<li>Export simbols/ordinals (if available)</li>
<li>Security info (if available)</li>
<li>Resources by types</li>
<li>Type libraries (if available)</li>
<li>Type library registration information (if available)</li>
<li>Manifest (if available)</li>
<li>System info (CPU model/stepping/futures, mb manufacturer/model, some OS properties)</li>
</ul>
Examples:
Self explore screenshots:
<p><img style="float:left;overflow: auto;" src=".\Images\PE32Viewer.dll.01.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\PE32Viewer.dll.02.jpg.png" height="595" width="447"></p>
<p><img style="float:left;overflow: auto;" src=".\Images\PE32Viewer.dll.03.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\PE32Viewer.dll.04.jpg.png" height="595" width="447"></p>
<p><img style="float:left;overflow: auto;" src=".\Images\PE32Viewer.dll.05.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\PE32Viewer.dll.06.jpg.png" height="595" width="447"></p>
<p><img style="float:left;overflow: auto;" src=".\Images\PE32Viewer.dll.07.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\PE32Viewer.dll.08.jpg.png" height="595" width="447"></p>
<p style="width:50%;"><img style="margin: auto;" src=".\Images\PE32Viewer.dll.09.jpg.png" height="595" width="447"></p>
Windows system files exploring screenshots ... accessibilitycpl.dll
<p><img style="float:left;overflow: auto;" src=".\Images\accessibilitycpl.dll.01.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\accessibilitycpl.dll.02.jpg.png" height="595" width="447"></p>
<p><img style="float:left;overflow: auto;" src=".\Images\accessibilitycpl.dll.04.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\accessibilitycpl.dll.05.jpg.png" height="595" width="447"></p>
... aclui.dll
<p><img style="float:left;overflow: auto;" src=".\Images\aclui.dll.01.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\aclui.dll.02.jpg.png" height="595" width="447"></p>
... CertEnroll.dll
<p style="width:50%;"><img style="margin: auto;" src=".\Images\PE32Viewer.dll.09.jpg.png" height="595" width="447"></p>
... win32kbase.sys
<p><img style="float:left;overflow: auto;" src=".\Images\win32kbase.sys.01.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\win32kbase.sys.02.jpg.png" height="595" width="447"></p>
<p><img style="float:left;overflow: auto;" src=".\Images\win32kbase.sys.03.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\win32kbase.sys.04.jpg.png" height="595" width="447"></p>
... win32kfull.sys
<p><img style="float:left;overflow: auto;" src=".\Images\win32kfull.sys.01.jpg.png" height="595" width="447"><img style="float:right;overflow: auto;" src=".\Images\win32kfull.sys.02.jpg.png" height="595" width="447"></p>
<p style="width:50%;"><img style="margin: auto;" src=".\Images\win32kfull.sys.03.jpg.png" height="595" width="447"></p>
