# Plots

## Design

### DMML nodes

- [vtkDMMLTableNode](https://apidocs.slicer.org/master/classvtkDMMLTableNode.html): Table node stores values that specify data point positions or bar heights in the plots.
- [vtkDMMLPlotSeriesNode](https://apidocs.slicer.org/master/classvtkDMMLPlotSeriesNode.html): Defines a data series by referring to a table node and column name(s) for X and Y axes and labels.
  - It also defines display properties, such as plot type, color, line style.
  - Line and bar plots only require Y axis (points along X axis are equally spaced), scatter plots require two input data columns, for X and Y axes.
- [vtkDMMLPlotChartNode](https://apidocs.slicer.org/master/classvtkDMMLPlotChartNode.html): Specifies which data series need to be shown in the chart.
  - Also contains global display properties for the chart, such as titles and font style.
- [vtkDMMLPlotViewNode](https://apidocs.slicer.org/master/classvtkDMMLPlotViewNode.html): Specifies which chart is to be displayed in the plot view and how the user can interact with it.
    - There has to be exactly one plot view node for each plot view widget. This class can not be created or copied unless is connected with a plot view.

### Widgets

- [qDMMLPlotView](https://apidocs.slicer.org/master/classqDMMLPlotView.html): Displays a plot. It can be embedded into a module user interface.
- [qDMMLPlotWidget](https://apidocs.slicer.org/master/classqDMMLPlotWidget.html): Displays a plot and in a popup window a plot view controller widget.
- [qDMMLPlotViewControllerWidget](https://apidocs.slicer.org/master/classqDMMLPlotViewControllerWidget.html): plot view controller widget.
- [qDMMLPlotSeriesPropertiesWidget](https://apidocs.slicer.org/master/classqDMMLPlotSeriesPropertiesWidget.html): Display/edit properties of a plot series node.
- [qDMMLPlotChartPropertiesWidget](https://apidocs.slicer.org/master/classqDMMLPlotChartPropertiesWidget.html): Display/edit properties of a plot series node.
- [qDMMLPlotViewControllerWidget](https://apidocs.slicer.org/master/classqDMMLPlotViewControllerWidget.html): Display/edit properties of a plot view node.

### Signals

qDMMLPlotView objects provide `dataSelected(vtkStringArray* dmmlPlotSeriesIDs, vtkCollection* selectionCol)` signal that allow modules to respond to user interactions with the Plot canvas. The signal is emitted when a data point or more has been selected. Returns the series node IDs and a list of selected point IDs (as a collection of `vtkIdTypeArray` objects).

Python API example:

```python
# Switch to a layout that contains a plot view to create a plot widget
layoutManager = cjyx.app.layoutManager()
layoutWithPlot = cjyx.modules.plots.logic().GetLayoutWithPlot(layoutManager.layout)
layoutManager.setLayout(layoutWithPlot)

# Select chart in plot view
plotWidget = layoutManager.plotWidget(0)
plotViewNode = plotWidget.dmmlPlotViewNode()

# Add a PlotCharNode 
# plotViewNode.SetPlotChartNodeID(''PlotChartNode''.GetID())

# Print selected point IDs
def onDataSelected(dmmlPlotDataIDs, selectionCol):
    print("Selection changed:")
    for selectionIndex in range(dmmlPlotDataIDs.GetNumberOfValues()):
        pointIdList = []
        pointIds = selectionCol.GetItemAsObject(selectionIndex)
        for pointIndex in range(pointIds.GetNumberOfValues()):
            pointIdList.append(pointIds.GetValue(pointIndex))
        print("  {0}: {1}".format(dmmlPlotDataIDs.GetValue(selectionIndex), pointIdList))


# Connect the signal with a slot ''onDataSelected''
plotView = plotWidget.plotView()
plotView.connect("dataSelected(vtkStringArray*, vtkCollection*)", self.onDataSelected) 
```

## Examples

Examples for common DICOM operations are provided in the [script repository](../script_repository.md#plots).
