/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMMLLogic includes
#include "vtkDMMLLayoutLogic.h"

// DMML includes
#include "vtkDMMLColors.h"
#include "vtkDMMLLayoutNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLViewNode.h"
#include "vtkDMMLTableViewNode.h"
#include "vtkDMMLPlotViewNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkXMLDataElement.h>

// STD includes
#include <cassert>
#include <sstream>

// Standard layouts definitions
//
// CompareView layouts are defined programmatically in the method
// UpdateCompareViewLayoutDefinitions()
//
const char* conventionalView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "  <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "    <property name=\"viewlabel\" action=\"default\">1</property>"
  "  </view>"
  " </item>"
  " <item splitSize=\"500\">"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourUpView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "     <property name=\"viewlabel\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* oneUp3DView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "   <property name=\"viewlabel\" action=\"default\">1</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* threeDTableView =
  "<layout type=\"horizontal\" split=\"true\">"
  " <item splitSize=\"500\">"
  "  <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "   <property name=\"viewlabel\" action=\"default\">1</property>"
  "  </view>"
  " </item>"
  " <item splitSize=\"500\">"
  "  <view class=\"vtkDMMLTableViewNode\" singletontag=\"TableView1\">"
  "  <property name=\"viewlabel\" action=\"default\">T</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* oneUpRedView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "   <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "  </view>"
  " </item>"
  "</layout>";
const char* oneUpYellowView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "   <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "  </view>"
  " </item>"
  "</layout>";
const char* oneUpGreenView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "   <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* tabbed3DView =
  "<layout type=\"tab\">"
  " <item multiple=\"true\">"
  "  <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "   <property name=\"viewlabel\" action=\"default\">1</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* tabbedSliceView =
  "<layout type=\"tab\">"
  " <item multiple=\"true\">"
  "  <view class=\"vtkDMMLSliceNode\">"
  "   <property name=\"viewlabel\" action=\"default\">1</property>"
  "   <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* dual3DView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "     <property name=\"viewlabel\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"2\" type=\"secondary\">"
  "     <property name=\"viewlabel\" action=\"default\">2</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item splitSize=\"500\">"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* conventionalWidescreenView =
  "<layout type=\"horizontal\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "  <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "   <property name=\"viewlabel\" action=\"default\">1</property>"
  "  </view>"
  " </item>"
  " <item splitSize=\"300\">"
  "  <layout type=\"vertical\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* triple3DEndoscopyView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "  <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "   <property name=\"viewlabel\" action=\"default\">1</property>"
  "  </view>"
  " </item>"
  " <item splitSize=\"500\">"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"2\" type=\"secondary\">"
  "     <property name=\"viewlabel\" action=\"default\">2</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"3\" type=\"endoscopy\">"
  "     <property name=\"viewlabel\" action=\"default\">3</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* threeOverThreeView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red+\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R+</property>"
  "     <property name=\"viewcolor\" action=\"default\">#f9a99f</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green+\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G+</property>"
  "     <property name=\"viewcolor\" action=\"default\">#c6e0b8</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow+\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y+</property>"
  "     <property name=\"viewcolor\" action=\"default\">#f6e9a2</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourOverFourView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "     <property name=\"viewlabel\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLViewNode\" singletontag=\"1+\" type=\"secondary\">"
  "     <property name=\"viewlabel\" action=\"default\">1+</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red+\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R+</property>"
  "     <property name=\"viewcolor\" action=\"default\">#f9a99f</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green+\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G+</property>"
  "     <property name=\"viewcolor\" action=\"default\">#c6e0b8</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow+\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y+</property>"
  "     <property name=\"viewcolor\" action=\"default\">#f6e9a2</property>"
  "     <property name=\"viewgroup\" action=\"default\">1</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* conventionalPlotView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "   <layout type=\"horizontal\">"
  "     <item>"
  "      <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "       <property name=\"viewlabel\" action=\"default\">1</property>"
  "      </view>"
  "     </item>"
  "     <item>"
  "      <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView1\">"
  "       <property name=\"viewlabel\" action=\"default\">P</property>"
  "      </view>"
  "     </item>"
  "   </layout>"
  " </item>"
  " <item splitSize=\"500\">"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourUpPlotView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView1\">"
  "     <property name=\"viewlabel\" action=\"default\">P</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourUpPlotTableView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "  <layout type=\"vertical\">"
  "   <item>"
  "    <layout type=\"horizontal\">"
  "     <item>"
  "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "       <property name=\"orientation\" action=\"default\">Axial</property>"
  "       <property name=\"viewlabel\" action=\"default\">R</property>"
  "       <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "      </view>"
  "     </item>"
  "     <item>"
  "      <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView1\">"
  "       <property name=\"viewlabel\" action=\"default\">P</property>"
  "      </view>"
  "     </item>"
  "    </layout>"
  "   </item>"
  "   <item>"
  "    <layout type=\"horizontal\">"
  "     <item>"
  "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "       <property name=\"orientation\" action=\"default\">Coronal</property>"
  "       <property name=\"viewlabel\" action=\"default\">G</property>"
  "       <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "      </view>"
  "     </item>"
  "     <item>"
  "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "       <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "       <property name=\"viewlabel\" action=\"default\">Y</property>"
  "       <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "      </view>"
  "     </item>"
  "    </layout>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item splitSize=\"300\">"
  "  <view class=\"vtkDMMLTableViewNode\" singletontag=\"TableView1\">"
  "    <property name=\"viewlabel\" action=\"default\">T</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* oneUpPlotView =
  "<layout type=\"horizontal\">"
  " <item>"
  "    <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView1\">"
  "     <property name=\"viewlabel\" action=\"default\">P</property>"
  "    </view>"
  " </item>"
  "</layout>";

const char* twoOverTwoView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice4\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">4</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* threeOverThreePlotView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView1\">"
  "     <property name=\"viewlabel\" action=\"default\">P</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView2\">"
  "     <property name=\"viewlabel\" action=\"default\">P2</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLPlotViewNode\" singletontag=\"PlotView3\">"
  "     <property name=\"viewlabel\" action=\"default\">P3</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourUpTableView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item splitSize=\"500\">"
  "  <layout type=\"vertical\">"
  "   <item>"
  "    <layout type=\"horizontal\">"
  "     <item>"
  "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "       <property name=\"orientation\" action=\"default\">Axial</property>"
  "       <property name=\"viewlabel\" action=\"default\">R</property>"
  "       <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "      </view>"
  "     </item>"
  "     <item>"
  "      <view class=\"vtkDMMLViewNode\" singletontag=\"1\">"
  "       <property name=\"viewlabel\" action=\"default\">1</property>"
  "      </view>"
  "     </item>"
  "    </layout>"
  "   </item>"
  "   <item>"
  "    <layout type=\"horizontal\">"
  "     <item>"
  "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "       <property name=\"orientation\" action=\"default\">Coronal</property>"
  "       <property name=\"viewlabel\" action=\"default\">G</property>"
  "       <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "      </view>"
  "     </item>"
  "     <item>"
  "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "       <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "       <property name=\"viewlabel\" action=\"default\">Y</property>"
  "       <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "      </view>"
  "     </item>"
  "    </layout>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item splitSize=\"300\">"
  "  <view class=\"vtkDMMLTableViewNode\" singletontag=\"TableView1\">"
  "    <property name=\"viewlabel\" action=\"default\">T</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* sideBySideView =
  "<layout type=\"horizontal\">"
  "  <item>"
  "   <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "    <property name=\"orientation\" action=\"default\">Axial</property>"
  "    <property name=\"viewlabel\" action=\"default\">R</property>"
  "    <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "   </view>"
  "  </item>"
  "  <item>"
  "   <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "    <property name=\"orientation\" action=\"default\">Axial</property>"
  "    <property name=\"viewlabel\" action=\"default\">Y</property>"
  "    <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "   </view>"
  "  </item>"
  "</layout>";

const char* fourByThreeSliceView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice4\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">4</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice5\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">5</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice6\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">6</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice7\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">7</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice8\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">8</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice9\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">9</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice10\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">10</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice11\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">11</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice12\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">12</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourByTwoSliceView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice4\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">4</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice5\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">5</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice6\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">6</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice7\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">7</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice8\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">8</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fiveByTwoSliceView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice4\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">4</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice5\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">5</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice6\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">6</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice7\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">7</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice8\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">8</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice9\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">9</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice10\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">10</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* threeByThreeSliceView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">R</property>"
  "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">Y</property>"
  "     <property name=\"viewcolor\" action=\"default\">#EDD54C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">G</property>"
  "     <property name=\"viewcolor\" action=\"default\">#6EB04B</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice4\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">4</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice5\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">5</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice6\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">6</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice7\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">7</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice8\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">8</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Slice9\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "     <property name=\"viewlabel\" action=\"default\">9</property>"
  "     <property name=\"viewcolor\" action=\"default\">#8C8C8C</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLLayoutLogic);

//----------------------------------------------------------------------------
vtkDMMLLayoutLogic::vtkDMMLLayoutLogic()
{
  this->LayoutNode = nullptr;
  this->LastValidViewArrangement = vtkDMMLLayoutNode::CjyxLayoutNone;
  this->ViewNodes = vtkCollection::New();
  this->ConventionalLayoutRootElement = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLLayoutLogic::~vtkDMMLLayoutLogic()
{
  this->SetLayoutNode(nullptr);

  this->ViewNodes->Delete();
  this->ViewNodes = nullptr;
  if (this->ConventionalLayoutRootElement)
    {
    this->ConventionalLayoutRootElement->Delete();
    }
}

//------------------------------------------------------------------------------
void vtkDMMLLayoutLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UnobserveDMMLScene()
{
  this->SetLayoutNode(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UpdateFromDMMLScene()
{
  vtkDebugMacro("vtkDMMLLayoutLogic::UpdateFromDMMLScene: got a NewScene event "
                << this->GetProcessingDMMLSceneEvent());
  // Create default 3D view + slice views
  this->UpdateViewNodes();
  // Create/Retrieve Layout node
  this->UpdateLayoutNode();
  // Restore the layout to its old state after importing a scene
  // TBD: check on GetIsUpdating() should be enough
  if (this->LayoutNode->GetViewArrangement() == vtkDMMLLayoutNode::CjyxLayoutNone
      && !this->GetDMMLScene()->IsBatchProcessing()
      //&& (this->GetProcessingDMMLSceneEvent() == vtkDMMLScene::EndCloseEvent
      //    && !this->GetDMMLScene()->IsBatchProcessing())
      //    || this->GetProcessingDMMLSceneEvent() == vtkDMMLScene::EndImportEvent)
      )
    {
    this->LayoutNode->SetViewArrangement(this->LastValidViewArrangement);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::OnDMMLNodeModified(vtkDMMLNode* vtkNotUsed(node))
{
  this->UpdateFromLayoutNode();
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::OnDMMLSceneStartRestore()
{
    this->UnobserveDMMLScene();
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::OnDMMLSceneEndRestore()
{
    this->UpdateLayoutNode();
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkDMMLLayoutLogic: " << this->GetClassName() << "\n";
  os << indent << "LayoutNode:         " << (this->LayoutNode ? this->LayoutNode->GetID() : "none") << "\n";
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UpdateViewNodes()
{
  if (!this->GetDMMLScene())
    {
    return;
    }
  if (this->ConventionalLayoutRootElement == nullptr)
    {
    // vtkDMMLLayoutLogic is responsible for the returned vtkXMLDataElement
    // pointer, this is why we delete it in the ~vtkDMMLLayoutLogic()
    this->ConventionalLayoutRootElement = vtkDMMLLayoutNode::ParseLayout(conventionalView);
    }
  this->CreateMissingViews(this->ConventionalLayoutRootElement);
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UpdateLayoutNode()
{
  if (!this->GetDMMLScene())
    {
    this->SetLayoutNode(nullptr);
    return;
    }
  if (this->LayoutNode && this->LayoutNode->GetScene() == this->GetDMMLScene())
    {
    this->UpdateFromLayoutNode();
    return;
    }
  vtkDMMLLayoutNode* sceneLayoutNode = vtkDMMLLayoutNode::SafeDownCast(
    this->GetDMMLScene()->GetFirstNodeByClass("vtkDMMLLayoutNode"));
  if (sceneLayoutNode)
    {
    this->SetLayoutNode(sceneLayoutNode);
    }
  else
    {
    sceneLayoutNode = vtkDMMLLayoutNode::New();

    // we want to set the node to the logic before adding it into the scene, in
    // case an object listens to the scene node added event and query the logic
    // about the layout node we need to be ready before it happens.
    this->SetLayoutNode(sceneLayoutNode);
    // we set the view after the layouts have been registered
    sceneLayoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutInitialView);

    // the returned value of vtkDMMLScene::AddNode can be different from its
    // input when the input is a singleton node (vtkDMMLNode::SingletonTag is 1)
    // As we observe the DMML scene, this->DMMLLayoutNode will be set in
    // onNodeAdded
    vtkDMMLNode * nodeCreated = this->GetDMMLScene()->AddNode(sceneLayoutNode);
    // as we checked that there was no vtkDMMLLayoutNode in the scene, the
    // returned node by vtkDMMLScene::AddNode() should be layoutNode
    if (nodeCreated != sceneLayoutNode)
      {
      vtkWarningMacro("Error when adding layout node into the scene");
      }
    sceneLayoutNode->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UpdateFromLayoutNode()
{
  if (this->LayoutNode &&
      this->LayoutNode->GetViewArrangement() != vtkDMMLLayoutNode::CjyxLayoutNone)
    {
    this->LastValidViewArrangement = this->LayoutNode->GetViewArrangement();
    }
  this->UpdateCompareViewLayoutDefinitions();
  this->CreateMissingViews();
  this->UpdateViewCollectionsFromLayout();
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UpdateCompareViewLayoutDefinitions()
{
  // std::cerr << "UpdateCompareViewLayoutDefinitions" << std::endl;

  if (!this->LayoutNode)
    {
    return;
    }

  int wasModifying = this->LayoutNode->StartModify();

  // Horizontal compare viewers
  std::stringstream compareView;
  compareView << "<layout type=\"vertical\" split=\"true\" >"
    " <item>"
    "  <layout type=\"horizontal\">"
    "   <item>"
    "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
    "     <property name=\"orientation\" action=\"default\">Axial</property>"
    "     <property name=\"viewlabel\" action=\"default\">R</property>"
    "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
    "    </view>"
    "   </item>"
    "   <item>"
    "    <view class=\"vtkDMMLViewNode\" singletontag=\"1\"/>"
    "   </item>"
    "  </layout>"
    " </item>"
    " <item>"
    "  <layout type=\"vertical\">";

  for (int i=1; i<=this->LayoutNode->GetNumberOfCompareViewRows(); ++i)
    {
    compareView <<
      "   <item>"
      "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Compare"<< i << "\">"
      "     <property name=\"orientation\" action=\"default\">Axial</property>"
      "     <property name=\"viewlabel\" action=\"default\">" << i << "</property>"
      "     <property name=\"viewcolor\" action=\"default\">#E17012</property>"
      "     <property name=\"lightboxrows\" action=\"default\">1</property>"
      "     <property name=\"lightboxcolumns\" action=\"default\">" << this->LayoutNode->GetNumberOfCompareViewLightboxColumns() << "</property>"
      "     <property name=\"lightboxrows\" action=\"relayout\">1</property>"
      "     <property name=\"lightboxcolumns\" action=\"relayout\">" << this->LayoutNode->GetNumberOfCompareViewLightboxColumns() << "</property>"
      "    </view>"
      "   </item>";
      }
  compareView <<
    "  </layout>"
    " </item>"
    "</layout>";

  if (this->LayoutNode->IsLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareView))
    {
    this->LayoutNode->SetLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareView,
                                           compareView.str().c_str());
    }
  else
    {
    this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareView,
                                           compareView.str().c_str());
    }

  // Vertical compare viewers
  std::stringstream compareWidescreenView;
  compareWidescreenView <<   "<layout type=\"horizontal\" split=\"true\" >"
    " <item>"
    "  <layout type=\"vertical\">"
    "   <item>"
    "    <view class=\"vtkDMMLViewNode\" singletontag=\"1\"/>"
    "   </item>"
    "   <item>"
    "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
    "     <property name=\"orientation\" action=\"default\">Axial</property>"
    "     <property name=\"viewlabel\" action=\"default\">R</property>"
    "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
    "    </view>"
    "   </item>"
    "  </layout>"
    " </item>"
    " <item>"
    "  <layout type=\"horizontal\">";

  for (int i=1; i <= this->LayoutNode->GetNumberOfCompareViewColumns(); ++i)
    {
    compareWidescreenView <<
      "   <item>"
      "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Compare"<< i<< "\">"
      "     <property name=\"orientation\" action=\"default\">Axial</property>"
      "     <property name=\"viewlabel\" action=\"default\">" << i << "</property>"
      "     <property name=\"viewcolor\" action=\"default\">#E17012</property>"
      "     <property name=\"lightboxrows\" action=\"default\">" << this->LayoutNode->GetNumberOfCompareViewLightboxRows() << "</property>"
      "     <property name=\"lightboxcolumns\" action=\"default\">1</property>"
      "     <property name=\"lightboxrows\" action=\"relayout\">" << this->LayoutNode->GetNumberOfCompareViewLightboxRows() << "</property>"
      "     <property name=\"lightboxcolumns\" action=\"relayout\">1</property>"
      "    </view>"
      "   </item>";
    }
  compareWidescreenView <<
    "  </layout>"
    " </item>"
    "</layout>";

  if (this->LayoutNode->IsLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareWidescreenView))
    {
    this->LayoutNode->SetLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareWidescreenView,
                                        compareWidescreenView.str().c_str());
    }
  else
    {
    this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareWidescreenView,
                                        compareWidescreenView.str().c_str());
    }

  // Grid compare viewers
  std::stringstream compareViewGrid;
  compareViewGrid << "<layout type=\"vertical\" split=\"true\" >"
    " <item>"
    "  <layout type=\"horizontal\">"
    "   <item>"
    "    <view class=\"vtkDMMLSliceNode\" singletontag=\"Red\">"
    "     <property name=\"orientation\" action=\"default\">Axial</property>"
    "     <property name=\"viewlabel\" action=\"default\">R</property>"
    "     <property name=\"viewcolor\" action=\"default\">#F34A33</property>"
    "    </view>"
    "   </item>"
    "   <item>"
    "    <view class=\"vtkDMMLViewNode\" singletontag=\"1\"/>"
    "   </item>"
    "  </layout>"
    " </item>"
    " <item>"
    "  <layout type=\"vertical\">";

  for (int i=1, k=1; i<=this->LayoutNode->GetNumberOfCompareViewRows(); ++i)
    {
    compareViewGrid <<
      "   <item>"
      "    <layout type=\"horizontal\">";
    for (int j=1; j <= this->LayoutNode->GetNumberOfCompareViewColumns();
         ++j,++k)
      {
      compareViewGrid <<
        "     <item>"
        "      <view class=\"vtkDMMLSliceNode\" singletontag=\"Compare"<< k << "\">"
        "       <property name=\"orientation\" action=\"default\">Axial</property>"
        "       <property name=\"viewlabel\" action=\"default\">" << k << "</property>"
        "       <property name=\"viewcolor\" action=\"default\">#E17012</property>"
        "       <property name=\"lightboxrows\" action=\"default\">1</property>"
        "       <property name=\"lightboxcolumns\" action=\"default\">1</property>"
        "       <property name=\"lightboxrows\" action=\"relayout\">1</property>"
        "       <property name=\"lightboxcolumns\" action=\"relayout\">1</property>"
        "      </view>"
        "     </item>";
      }
    compareViewGrid <<
      "     </layout>"
      "    </item>";
    }
  compareViewGrid <<
    "  </layout>"
    " </item>"
    "</layout>";

  if (this->LayoutNode->IsLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareGridView))
    {
    this->LayoutNode->SetLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareGridView,
                                           compareViewGrid.str().c_str());
    }
  else
    {
    this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutCompareGridView,
                                           compareViewGrid.str().c_str());
    }
  this->LayoutNode->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::SetLayoutNode(vtkDMMLLayoutNode* layoutNode)
{
  if (this->LayoutNode == layoutNode)
    {
    return;
    }
  // vtkDMMLLayoutLogic needs to receive the ModifiedEvent before anyone (in
  // particular qDMMLLayoutManager). It should be the case as vtkDMMLLayoutLogic
  // is the first to add an observer to it. However VTK wrongly calls the
  // firstly added observer after all the other observer are called. So we need
  // to enforce, using a priority, that it is first. The observer manager
  // can't give control over the priority so we need to do the observation
  // manually.
  //this->GetDMMLNodesObserverManager()->SetAndObserveObject(
  //  vtkObjectPointer(&this->LayoutNode), layoutNode);

  if (this->LayoutNode)
    {
    this->LayoutNode->RemoveObservers(vtkCommand::ModifiedEvent, this->GetDMMLNodesCallbackCommand());
    }
  this->LayoutNode = layoutNode;
  if (this->LayoutNode)
    {
    this->LayoutNode->AddObserver(vtkCommand::ModifiedEvent, this->GetDMMLNodesCallbackCommand(), 10.);
    }

  // Add default layouts only the first time (when the layout node is set)
  this->AddDefaultLayouts();
  if (this->LayoutNode)
    {
    // Maybe the vtkDMMLLayoutNode::ViewArrangement was set before the
    // layout descriptions were added into the node.
    // UpdateCurrentLayoutDescription needs to be called.
    this->LayoutNode->SetViewArrangement(this->LayoutNode->GetViewArrangement());
    }
  this->OnDMMLNodeModified(this->LayoutNode); //this->UpdateFromLayoutNode();
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::AddDefaultLayouts()
{
  if (!this->LayoutNode)
    {
    return;
    }
  int wasModifying = this->LayoutNode->StartModify();
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutInitialView,
                                         fourUpView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutDefaultView,
                                         conventionalView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutConventionalView,
                                         conventionalView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourUpView,
                                         fourUpView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutOneUp3DView,
                                         oneUp3DView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayout3DTableView,
                                         threeDTableView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView,
                                         oneUpRedView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutOneUpYellowSliceView,
                                         oneUpYellowView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutOneUpGreenSliceView,
                                         oneUpGreenView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutTabbed3DView,
                                         tabbed3DView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutTabbedSliceView,
                                         tabbedSliceView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutThreeOverThreeView,
                                         threeOverThreeView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourOverFourView,
                                         fourOverFourView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutDual3DView,
                                         dual3DView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutConventionalWidescreenView,
                                         conventionalWidescreenView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutTriple3DEndoscopyView,
                                         triple3DEndoscopyView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutConventionalPlotView,
                                         conventionalPlotView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourUpPlotView,
                                         fourUpPlotView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourUpPlotTableView,
                                         fourUpPlotTableView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutOneUpPlotView,
                                         oneUpPlotView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutTwoOverTwoView,
                                         twoOverTwoView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutThreeOverThreePlotView,
                                         threeOverThreePlotView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourUpTableView,
                                         fourUpTableView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutSideBySideView,
                                         sideBySideView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourByThreeSliceView,
                                         fourByThreeSliceView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFourByTwoSliceView,
                                         fourByTwoSliceView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutFiveByTwoSliceView,
                                         fiveByTwoSliceView);
  this->LayoutNode->AddLayoutDescription(vtkDMMLLayoutNode::CjyxLayoutThreeByThreeSliceView,
                                         threeByThreeSliceView);
  // add the CompareView modes which are defined programmatically
  this->UpdateCompareViewLayoutDefinitions();
  this->LayoutNode->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLLayoutLogic::GetViewFromElement(vtkXMLDataElement* element)
{
  return this->GetViewFromAttributes(this->GetViewElementAttributes(element));
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLLayoutLogic::CreateViewFromAttributes(const ViewAttributes& attributes)
{
  // filter on the class name, that remove a lot of options
  ViewAttributes::const_iterator it = attributes.find(std::string("class"));
  ViewAttributes::const_iterator end = attributes.end();
  if (it == end)
    {
    return nullptr;
    }
  const std::string& className = it->second;
  vtkDMMLNode* node = this->GetDMMLScene()->CreateNodeByClass(className.c_str());
  it = attributes.find(std::string("type"));
  if (it != end)
    {
    const std::string& type = it->second;
    node->SetAttribute("ViewType", type.c_str());
    }
  vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    it = attributes.find(std::string("singletontag"));
    if (it != end)
      {
      const std::string& singletonTag = it->second;
      viewNode->SetLayoutName(singletonTag.c_str());
      }
    std::string name = std::string(viewNode->GetLayoutName());
    // Maintain backward compatibility
    if (!viewNode->IsA("vtkDMMLSliceNode")
        && !viewNode->IsA("vtkDMMLTableViewNode") && !viewNode->IsA("vtkDMMLPlotViewNode"))
      {
      name = std::string("View") + name;
      }
    viewNode->SetName(name.c_str());
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::ApplyProperties(const ViewProperties& properties, vtkDMMLNode* view, const std::string& action)
{
  for (unsigned int i = 0; i < properties.size(); ++i)
    {
    ViewProperty property = properties[i];
    ViewProperty::const_iterator it = property.find("action");
    if (it != property.end() &&
        it->second != action)
      {
      continue;
      }
    this->ApplyProperty(property, view);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::ApplyProperty(const ViewProperty& property, vtkDMMLNode* view)
{
  std::string value;
  ViewProperty::const_iterator it = property.find("value");
  if (it != property.end())
    {
    value = it->second;
    }
  it = property.find("name");
  if (it == property.end())
    {
    vtkWarningMacro("Invalid property, no name given.");
    return;
    }

  const std::string name = it->second;
  // Orientation
  if (name == std::string("orientation"))
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(view);
    if (!sliceNode)
      {
      vtkWarningMacro("Invalid orientation property.");
      return;
      }
    sliceNode->SetDefaultOrientation(value.c_str());
    sliceNode->SetOrientation(value.c_str());
    }
  // ViewGroup
  if (name == std::string("viewgroup"))
    {
    vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(view);
    if (!viewNode)
      {
      vtkWarningMacro("Invalid view group property.");
      return;
      }
    std::stringstream ss;
    int n;
    ss << value;
    ss >> n;
    viewNode->SetViewGroup(n);
    }
  // LayoutLabel
  if (name == std::string("viewlabel"))
    {
    vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(view);
    if (!viewNode)
      {
      vtkWarningMacro("Invalid view label property.");
      return;
      }
    viewNode->SetLayoutLabel(value.c_str());
    }
  // ViewColor
  if (name == std::string("viewcolor"))
    {
    vtkDMMLAbstractViewNode* viewNode = vtkDMMLAbstractViewNode::SafeDownCast(view);
    if (!viewNode)
      {
      vtkWarningMacro("Invalid view color property.");
      return;
      }
    double color[3];
    vtkDMMLColors::toRGBColor(value.c_str(), color);
    viewNode->SetLayoutColor(color);
    }
  // Lightbox
  if (name == std::string("lightboxrows"))
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(view);
    if (!sliceNode)
      {
      vtkWarningMacro("Invalid lightboxrows property.");
      return;
      }
    std::stringstream ss;
    int n;
    ss << value;
    ss >> n;
    sliceNode->SetLayoutGridRows(n);
    }
  if (name == std::string("lightboxcolumns"))
    {
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(view);
    if (!sliceNode)
      {
      vtkWarningMacro("Invalid lightboxcolumns property.");
      return;
      }
    std::stringstream ss;
    int n;
    ss << value;
    ss >> n;
    sliceNode->SetLayoutGridColumns(n);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::MaximizeView(vtkDMMLAbstractViewNode* viewToMaximize)
{
  int layout = vtkDMMLLayoutNode::CjyxLayoutMaximizedView;
  this->CreateMaximizedViewLayoutDescription( layout, viewToMaximize);
  vtkDMMLLayoutNode* layoutNode = this->GetLayoutNode();
  if (layoutNode)
    {
    layoutNode->SetViewArrangement(layout);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic
::CreateMaximizedViewLayoutDescription(int layout,
                                       vtkDMMLAbstractViewNode* viewToMaximize)
{
  vtkDMMLLayoutNode* layoutNode = this->GetLayoutNode();
  if (!layoutNode)
    {
    vtkErrorMacro( << "No layout node");
    }
  std::string layoutDescription = this->GetMaximizedViewLayoutDescription(viewToMaximize);
  if (layoutNode->IsLayoutDescription(layout))
    {
    layoutNode->SetLayoutDescription(layout, layoutDescription.c_str());
    }
  else
    {
    layoutNode->AddLayoutDescription(layout, layoutDescription.c_str());
    }
}

//----------------------------------------------------------------------------
std::string vtkDMMLLayoutLogic::GetMaximizedViewLayoutDescription(vtkDMMLAbstractViewNode* viewToMaximize)
{
  std::stringstream layoutDescription;
  layoutDescription <<
    "<layout type=\"horizontal\">"
    " <item>"
    "  <view class=\"" << viewToMaximize->GetClassName() << "\" "
    "singletontag=\"" << viewToMaximize->GetSingletonTag() << "\">"
    "  </view>"
    " </item>"
    "</layout>";
  return layoutDescription.str();
}

//----------------------------------------------------------------------------
vtkDMMLNode* vtkDMMLLayoutLogic::GetViewFromAttributes(const ViewAttributes& attributes)
{
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetViewsFromAttributes(attributes));
  if (nodes.GetPointer() == nullptr || nodes->GetNumberOfItems() == 0)
    {
    return nullptr;
    }
  return vtkDMMLNode::SafeDownCast(nodes->GetItemAsObject(0));
}

//----------------------------------------------------------------------------
vtkCollection* vtkDMMLLayoutLogic::GetViewsFromAttributes(const ViewAttributes& attributes)
{
  if (!this->GetDMMLScene())
    {
    return nullptr;
    }
  // filter on the class name to remove a lot of options.
  ViewAttributes::const_iterator it = attributes.find(std::string("class"));
  ViewAttributes::const_iterator end = attributes.end();
  if (it == end)
    {
    return nullptr;
    }
  const std::string& className = it->second;
  vtkCollection* nodes = this->GetDMMLScene()->GetNodesByClass(className.c_str());
  if (nodes == nullptr || nodes->GetNumberOfItems() == 0)
    {
    // It is not necessarily a bug to not find nodes. It just means they
    // are not in the scene and will be then created by CreateMissingViews()
    vtkDebugMacro("Couldn't find nodes matching class: " << className);
    if (nodes)
      {
      nodes->Delete();
      nodes = nullptr;
      }
    return nodes;
    }
  vtkCollectionSimpleIterator nodesIt;
  vtkDMMLNode* node;
  for (it = attributes.begin(); it != end; ++it)
    {
    nodes->InitTraversal(nodesIt);
    std::string attributeName = it->first;
    std::string attributeValue = it->second;
    if (attributeName == "class")
      {
      continue;
      }
    else if (attributeName == "singletontag")
      {
      for (;(node = vtkDMMLNode::SafeDownCast(nodes->GetNextItemAsObject(nodesIt)));)
        {
        std::string singletonTag =
          node->GetSingletonTag() ? node->GetSingletonTag() : "";
        if (attributeValue != singletonTag)
          {
          nodes->RemoveItem(node);
          }
        }
      if (nodes->GetNumberOfItems() > 1)
        {
        vtkWarningMacro("Found several nodes with a similar SingletonTag: " << attributeValue );
        // Did not find the node, return an empty list to trigger the
        // calling method, CreateMissingViews(), to create the appropriate node.
        nodes->RemoveAllItems();
        break;
        }
      assert(nodes->GetNumberOfItems() == 0 ||
             vtkDMMLNode::SafeDownCast(nodes->GetItemAsObject(0))->GetSingletonTag() == attributeValue);
      }
    else if (attributeName == "type")
      {
      if (className != "vtkDMMLViewNode")
        {
        continue;
        }
      for (;(node = vtkDMMLNode::SafeDownCast(nodes->GetNextItemAsObject(nodesIt)));)
        {
        std::string viewType =
          node->GetAttribute("ViewType") ? node->GetAttribute("ViewType") : "";

        if (attributeValue != viewType &&
            // if there is no viewType, it's a main view.
            !(attributeValue == "main" && viewType != std::string()))
          {
          nodes->RemoveItem(node);
          }
        }
      }
    // Add here specific codes to retrieve views
    }
  return nodes;
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::UpdateViewCollectionsFromLayout()
{
  if (!this->LayoutNode)
    {
    this->ViewNodes->RemoveAllItems();
    return;
    }
  this->ViewNodes->Delete();
  this->ViewNodes = this->GetViewsFromLayout(this->LayoutNode->GetLayoutRootElement());
}

//----------------------------------------------------------------------------
vtkCollection* vtkDMMLLayoutLogic::GetViewsFromLayout(vtkXMLDataElement* root)
{
  vtkCollection* views = vtkCollection::New();
  vtkXMLDataElement* viewElement = root;
  while ((viewElement = this->GetNextViewElement(viewElement)))
    {
    vtkDMMLNode* viewNode = this->GetViewFromElement(viewElement);
    if (!viewNode)
      {
      vtkWarningMacro("Can't find node for element: " << viewElement->GetName());
      viewElement->PrintXML(std::cerr, vtkIndent(0));
      }
    else
      {
      views->AddItem(viewNode);
      }
    }
  return views;
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::CreateMissingViews()
{
  this->CreateMissingViews(
    this->LayoutNode ? this->LayoutNode->GetLayoutRootElement() : nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLLayoutLogic::CreateMissingViews(vtkXMLDataElement* layoutRootElement)
{
  vtkXMLDataElement* viewElement = layoutRootElement;
  while ((viewElement = this->GetNextViewElement(viewElement)))
    {
    vtkDMMLNode* viewNode = this->GetViewFromElement(viewElement);
    if (viewNode)
      {
      // View already exists, just apply the "relayout" properties for
      // the layout
      ViewProperties properties = this->GetViewElementProperties(viewElement);
      this->ApplyProperties(properties, viewNode, "relayout");
      continue;
      }
    ViewAttributes attributes = this->GetViewElementAttributes(viewElement);
    viewNode = this->CreateViewFromAttributes(attributes);
    if (!viewNode)
      {
      vtkWarningMacro("Can't find node for element: " << viewElement->GetName());
      viewElement->PrintXML(std::cerr, vtkIndent(0));
      }
    // New View, apply the "default" properties for the layout.
    ViewProperties properties = this->GetViewElementProperties(viewElement);
    this->ApplyProperties(properties, viewNode, "default");
    this->GetDMMLScene()->AddNode(viewNode);
    viewNode->Delete();
    }
}

//----------------------------------------------------------------------------
/*
vtkCollection* vtkDMMLLayoutLogic::GetViewsFromLayoutDescription(const char * layoutDescription)
{
  vtkSmartPointer<vtkXMLDataElement> root;
  root.Take(this->ParseLayout(layoutDescription));
  return this->GetViewsFromLayout(this->LayoutNode->GetLayoutRootElement());
}
*/

//----------------------------------------------------------------------------
vtkDMMLLayoutLogic::ViewAttributes vtkDMMLLayoutLogic
::GetViewElementAttributes(vtkXMLDataElement* viewElement)const
{
  ViewAttributes attributes;
  assert(viewElement->GetName() == std::string("view"));
  for (int i = 0; i < viewElement->GetNumberOfAttributes(); ++i)
    {
    attributes[viewElement->GetAttributeName(i)] = viewElement->GetAttributeValue(i);
    }
  return attributes;
}

//----------------------------------------------------------------------------
vtkDMMLLayoutLogic::ViewProperties vtkDMMLLayoutLogic
::GetViewElementProperties(vtkXMLDataElement* viewElement)const
{
  ViewProperties properties;
  assert(viewElement->GetName() == std::string("view"));

  for (int i = 0; i < viewElement->GetNumberOfNestedElements(); ++i)
    {
    properties.push_back(this->GetViewElementProperty(viewElement->GetNestedElement(i)));
    }
  return properties;
}

//----------------------------------------------------------------------------
vtkDMMLLayoutLogic::ViewProperty vtkDMMLLayoutLogic
::GetViewElementProperty(vtkXMLDataElement* viewProperty)const
{
  ViewProperty property;
  assert(viewProperty->GetName() == std::string("property"));

  for (int i = 0; i < viewProperty->GetNumberOfAttributes(); ++i)
    {
    property[viewProperty->GetAttributeName(i)] = viewProperty->GetAttributeValue(i);
    }
  property["value"]  = viewProperty->GetCharacterData();
  return property;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* vtkDMMLLayoutLogic::GetNextViewElement(vtkXMLDataElement* viewElement)
{
  if (!viewElement)
    {
    return nullptr;
    }
  while (viewElement)
    {
    vtkXMLDataElement* nextViewElement = viewElement->LookupElementWithName("view");
    if (nextViewElement)
      {
      return nextViewElement;
      }
    viewElement = this->GetNextElement(viewElement);
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* vtkDMMLLayoutLogic::GetNextElement(vtkXMLDataElement* element)
{
  if (!element)
    {
    return nullptr;
    }
  // try it's sibling first
  vtkXMLDataElement* parent = element->GetParent();
  if (!parent)
    {
    return nullptr;
    }
  // find the index of the current element
  for (int i = 0; i < parent->GetNumberOfNestedElements() - 1; ++i)
    {
    if (element == parent->GetNestedElement(i))
      {
      // found, return the next element
      return parent->GetNestedElement(i+1);
      }
    }
  // the element didn't have any younger sibling, pick an uncle younger than parent
  return this->GetNextElement(parent);
}
