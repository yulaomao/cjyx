
#ifndef __qDMMLWidgetsExport_h
#define __qDMMLWidgetsExport_h

#if defined(WIN32) && !defined(qDMMLWidgets_STATIC)
 #if defined(qDMMLWidgets_EXPORTS)
  #define QDMML_WIDGETS_EXPORT __declspec( dllexport )
 #else
  #define QDMML_WIDGETS_EXPORT __declspec( dllimport )
 #endif
#else
 #define QDMML_WIDGETS_EXPORT
#endif

#endif //__qDMMLWidgetsExport_h
