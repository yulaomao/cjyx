import cjyx

#########################################################
#
#
comment = """

  DataProbeUtil holds utility functions required by the other
  classes

  Note: this needs to be a class so it can be reloaded

# TODO :
"""
#
#########################################################


class DataProbeUtil:

    def getParameterNode(self):
        """Get the DataProbe parameter node - a singleton in the scene"""
        node = self._findParameterNodeInScene()
        if not node:
            node = self._createParameterNode()
        return node

    def _findParameterNodeInScene(self):
        node = None
        size = cjyx.dmmlScene.GetNumberOfNodesByClass("vtkDMMLScriptedModuleNode")
        for i in range(size):
            n = cjyx.dmmlScene.GetNthNodeByClass(i, "vtkDMMLScriptedModuleNode")
            if n.GetModuleName() == "DataProbe":
                node = n
        return node

    def _createParameterNode(self):
        """create the DataProbe parameter node - a singleton in the scene
        This is used internally by getParameterNode - shouldn't really
        be called for any other reason.
        """
        node = cjyx.vtkDMMLScriptedModuleNode()
        node.SetSingletonTag("DataProbe")
        node.SetModuleName("DataProbe")
        # node.SetParameter( "label", "1" )
        cjyx.dmmlScene.AddNode(node)
        # Since we are a singleton, the scene won't add our node into the scene,
        # but will instead insert a copy, so we find that and return it
        node = self._findParameterNodeInScene()
        return node
