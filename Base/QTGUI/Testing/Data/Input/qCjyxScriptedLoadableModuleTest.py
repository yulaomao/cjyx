class qCjyxScriptedLoadableModuleTest:
    def __init__(self, parent):
        parent.title = "qCjyxScriptedLoadableModule Test"
        parent.categories = ["Testing"]
        parent.associatedNodeTypes = ["vtkDMMLModelNode", "vtkDMMLScalarVolumeNode"]
        parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)"]
        parent.helpText = """
    This module is used to test qCjyxScriptedLoadableModule and qCjyxScriptedLoadableModuleWidget classes.
    """
        parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc. and was partially funded by NIH grant 3P41RR013218-12S1
    """
        self.parent = parent

    def setup(self):
        self.parent.setProperty('setup_called_within_Python', True)


class qCjyxScriptedLoadableModuleTestWidget:
    def __init__(self, parent=None):
        self.parent = parent

    def setup(self):
        self.parent.setProperty('setup_called_within_Python', True)

    def enter(self):
        self.parent.setProperty('enter_called_within_Python', True)

    def exit(self):
        self.parent.setProperty('exit_called_within_Python', True)
