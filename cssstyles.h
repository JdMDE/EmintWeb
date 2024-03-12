#ifndef _CSS_STYLES_H
#define _CSS_STYLES_H

#define CSS_FOR_SCROLL_BARS  "QScrollBar:horizontal { border: 1px solid grey; background: #909090; height: 15px; margin: 0px 20px 0px 20px; }" \
  "QScrollBar::handle:horizontal { background: #004C99; min-width: 20px; }" \
  "QScrollBar::add-line:horizontal { border: 2px solid grey; background: #909090; width: 20px; subcontrol-position: right; subcontrol-origin: margin; }" \
  "QScrollBar::sub-line:horizontal { border: 2px solid grey; background: #909090; width: 20px; subcontrol-position: left; subcontrol-origin: margin; }" \
  "QScrollBar:left-arrow:horizontal, QScrollBar::right-arrow:horizontal { border: 2px solid grey; width: 4px; height: 4px; background: black; }" \
  "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }" \
  "QScrollBar:vertical { border: 1px solid grey; background: #909090; width: 15px; margin: 22px 0px 22px 0px; }" \
  "QScrollBar::handle:vertical { background: #004C99; min-height: 20px; }" \
  "QScrollBar::add-line:vertical { border: 2px solid grey; background: #909090; height: 20px; subcontrol-position: bottom; subcontrol-origin: margin; }" \
  "QScrollBar::sub-line:vertical { border: 2px solid grey; background: #909090; height: 20px; subcontrol-position: top; subcontrol-origin: margin; }" \
  "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { border: 2px solid grey; width: 4px; height: 3px; background: black; }" \
  "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"

#define CSS_FOR_MENUBAR  "QMenuBar { background-color: #606060; foreground-color: #000000; border: 1px solid black; spacing: 5px; }" \
   "QMenuBar::item { background-color: #C0C0C0; }" \
   "QMenuBar::item::selected { background: #D0D0D0; }" \
   "QMenuBar::item::pressed { background: #E0E0E0; }"
   
#define CSS_FOR_QMENU    "QMenu { background-color: #ABABAB; foreground-color: #000000; border: 1px solid black; }"

// This is not a Qt5 pure object but our class RenderArea, which is a subclass of QWidget.
#define CSS_FOR_RENDERAREA "* { background-color: #FFFFFF; foreground-color: #000000; border: 1px solid black; }"

#define CSS_FOR_COMBOBOX "QComboBox { border: 1px solid gray; border-radius: 3px; padding: 1px 18px 1px 3px; min-width: 6em; }" \
   "QComboBox:on { padding-top: 3px; padding-left: 4px; }" \
   "QComboBox:editable { background: white; }" \
   "QComboBox:!editable, QComboBox::drop-down:editable { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E1E1E1, stop: 0.4 #DDDDDD, stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3); }" \
   "QComboBox:!editable:on, QComboBox::drop-down:editable:on { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D3D3D3, stop: 0.4 #D8D8D8, stop: 0.5 #DDDDDD, stop: 1.0 #E1E1E1); }" \
   "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 15px; border-left-width: 1px; border-left-color: #909090; border-left-style: solid; border-top-right-radius: 3px; border-bottom-right-radius: 3px; }" \
   "QComboBox::down-arrow { border: 2px solid grey; width: 4px; height: 3px; background: black; }"
   
#define CSS_STYLE_FOR_COMBOBOX_VBAR "QScrollBar:vertical { border: 1px solid grey; background: #909090; width: 15px; margin: 22px 0px 22px 0px; }" \
  "QScrollBar::handle:vertical { background: #004C99; min-height: 20px; }" \
  "QScrollBar::add-line:vertical { border: 2px solid grey; background: #909090; height: 20px; subcontrol-position: bottom; subcontrol-origin: margin; }" \
  "QScrollBar::sub-line:vertical { border: 2px solid grey; background: #909090; height: 20px; subcontrol-position: top; subcontrol-origin: margin; }" \
  "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { border: 2px solid grey; width: 4px; height: 3px; background: black; }" \
  "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
  
#endif
