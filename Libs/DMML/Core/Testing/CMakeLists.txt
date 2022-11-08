set(KIT ${PROJECT_NAME})

set(EXTRA_INCLUDE "vtkDMMLDebugLeaksMacro.h\"\n\#include \"vtkTestingOutputWindow.h")
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();\nTESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDMMLBSplineTransformNodeTest1.cxx
  vtkDMMLCameraNodeTest1.cxx
  vtkDMMLClipModelsNodeTest1.cxx
  vtkDMMLColorNodeTest1.cxx
  vtkDMMLColorTableNodeTest1.cxx
  vtkDMMLColorTableStorageNodeTest1.cxx
  vtkDMMLCoreTestingUtilitiesTest.cxx
  vtkDMMLCrosshairNodeTest1.cxx
  vtkDMMLDiffusionImageVolumeNodeTest1.cxx
  vtkDMMLDiffusionTensorDisplayPropertiesNodeTest1.cxx
  vtkDMMLDiffusionTensorVolumeDisplayNodeTest1.cxx
  vtkDMMLDiffusionTensorVolumeNodeTest1.cxx
  vtkDMMLDiffusionTensorVolumeSliceDisplayNodeTest1.cxx
  vtkDMMLDiffusionWeightedVolumeDisplayNodeTest1.cxx
  vtkDMMLDiffusionWeightedVolumeNodeTest1.cxx
  vtkDMMLDisplayNodeTest1.cxx
  vtkDMMLDisplayableHierarchyNodeDisplayPropertiesTest.cxx
  vtkDMMLDisplayableHierarchyNodeTest1.cxx
  vtkDMMLDisplayableHierarchyNodeTest2.cxx
  vtkDMMLDisplayableHierarchyNodeTest3.cxx
  vtkDMMLDisplayableNodeTest1.cxx
  vtkDMMLGlyphableVolumeDisplayNodeTest1.cxx
  vtkDMMLGlyphableVolumeSliceDisplayNodeTest1.cxx
  vtkDMMLGridTransformNodeTest1.cxx
  vtkDMMLHierarchyNodeTest1.cxx
  vtkDMMLHierarchyNodeTest3.cxx
  vtkDMMLInteractionNodeTest1.cxx
  vtkDMMLLabelMapVolumeDisplayNodeTest1.cxx
  vtkDMMLLayoutNodeTest1.cxx
  vtkDMMLLinearTransformNodeEventsTest.cxx
  vtkDMMLLinearTransformNodeTest1.cxx
  vtkDMMLModelDisplayNodeTest1.cxx
  vtkDMMLModelHierarchyNodeTest1.cxx
  vtkDMMLModelNodeTest1.cxx
  vtkDMMLModelStorageNodeTest1.cxx
  vtkDMMLNRRDStorageNodeTest1.cxx
  vtkDMMLNodeTest1.cxx
  vtkDMMLNonlinearTransformNodeTest1.cxx
  vtkDMMLPETProceduralColorNodeTest1.cxx
  vtkDMMLPlotChartNodeTest1.cxx
  vtkDMMLPlotSeriesNodeTest1.cxx
  vtkDMMLPlotViewNodeTest1.cxx
  vtkDMMLProceduralColorNodeTest1.cxx
  vtkDMMLProceduralColorStorageNodeTest1.cxx
  vtkDMMLROIListNodeTest1.cxx
  vtkDMMLROINodeTest1.cxx
  vtkDMMLScalarVolumeDisplayNodeTest1.cxx
  vtkDMMLScalarVolumeNodeTest1.cxx
  vtkDMMLScalarVolumeNodeTest2.cxx
  vtkDMMLSceneAddSingletonTest.cxx
  vtkDMMLSceneBatchProcessTest.cxx
  vtkDMMLSceneIDTest.cxx
  vtkDMMLSceneImportIDConflictTest.cxx
  vtkDMMLSceneImportIDModelHierarchyConflictTest.cxx
  vtkDMMLSceneImportIDModelHierarchyParentIDConflictTest.cxx
  vtkDMMLSceneImportTest.cxx
  vtkDMMLSceneTest1.cxx
  vtkDMMLSceneTest2.cxx
  vtkDMMLSceneDefaultNodeTest.cxx
  # Disabled scene view tests for now - they will be fixed in upcoming commit
  # vtkDMMLSceneViewNodeImportSceneTest.cxx
  # vtkDMMLSceneViewNodeEventsTest.cxx
  # vtkDMMLSceneViewNodeRestoreSceneTest.cxx
  # vtkDMMLSceneViewNodeStoreSceneTest.cxx
  vtkDMMLSceneViewNodeTest1.cxx
  vtkDMMLSceneViewStorageNodeTest1.cxx
  vtkDMMLScriptedModuleNodeTest1.cxx
  vtkDMMLSegmentationStorageNodeTest1.cxx
  vtkDMMLSelectionNodeTest1.cxx
  vtkDMMLSliceCompositeNodeTest1.cxx
  vtkDMMLSliceNodeTest1.cxx
  vtkDMMLSnapshotClipNodeTest1.cxx
  vtkDMMLStorableNodeTest1.cxx
  vtkDMMLStorageNodeTest1.cxx
  vtkDMMLStreamingVolumeNodeTest1.cxx
  vtkDMMLTableNodeTest1.cxx
  vtkDMMLTableStorageNodeTest1.cxx
  vtkDMMLTableSQLiteStorageNodeTest.cxx
  vtkDMMLTableViewNodeTest1.cxx
  vtkDMMLTensorVolumeNodeTest1.cxx
  vtkDMMLTextNodeTest1.cxx
  vtkDMMLTextStorageNodeTest1.cxx
  vtkDMMLTransformableNodeReferenceSaveImportTest.cxx
  vtkDMMLTransformableNodeOnNodeReferenceAddTest.cxx
  vtkDMMLTransformDisplayNodeTest1.cxx
  vtkDMMLTransformNodeTest1.cxx
  vtkDMMLTransformStorageNodeTest1.cxx
  vtkDMMLTransformableNodeTest1.cxx
  vtkDMMLUnitNodeTest1.cxx
  vtkDMMLVectorVolumeDisplayNodeTest1.cxx
  vtkDMMLVectorVolumeNodeTest1.cxx
  vtkDMMLViewNodeTest1.cxx
  vtkDMMLVolumeArchetypeStorageNodeTest1.cxx
  vtkDMMLVolumeDisplayNodeTest1.cxx
  vtkDMMLVolumeHeaderlessStorageNodeTest1.cxx
  vtkDMMLVolumeNodeEventsTest.cxx
  vtkDMMLVolumeNodeTest1.cxx
  vtkDMMLdGEMRICProceduralColorNodeTest1.cxx
  vtkArchiveTest1.cxx
  vtkCodedEntryTest1.cxx
  vtkObserverManagerTest1.cxx
  vtkOrientedBSplineTransformTest1.cxx
  vtkOrientedGridTransformTest1.cxx
  vtkThinPlateSplineTransformTest1.cxx
  EXTRA_INCLUDE ${EXTRA_INCLUDE}
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} vtkDMMLSceneEventRecorder.cxx)
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#-----------------------------------------------------------------------------
set(DATAPATH "${CMAKE_CURRENT_SOURCE_DIR}/TestData")
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
simple_test( vtkDMMLBSplineTransformNodeTest1 )
simple_test( vtkDMMLCameraNodeTest1 )
simple_test( vtkDMMLClipModelsNodeTest1 )
simple_test( vtkDMMLColorNodeTest1 )
simple_test( vtkDMMLColorTableNodeTest1 ${TEMP})
simple_test( vtkDMMLColorTableStorageNodeTest1 )
simple_test( vtkDMMLCoreTestingUtilitiesTest )
simple_test( vtkDMMLCrosshairNodeTest1 )
simple_test( vtkDMMLdGEMRICProceduralColorNodeTest1 )
simple_test( vtkDMMLDiffusionImageVolumeNodeTest1 )
simple_test( vtkDMMLDiffusionTensorDisplayPropertiesNodeTest1 )
simple_test( vtkDMMLDiffusionTensorVolumeDisplayNodeTest1 )
simple_test( vtkDMMLDiffusionTensorVolumeNodeTest1 )
simple_test( vtkDMMLDiffusionTensorVolumeSliceDisplayNodeTest1 )
simple_test( vtkDMMLDiffusionWeightedVolumeDisplayNodeTest1 )
simple_test( vtkDMMLDiffusionWeightedVolumeNodeTest1 )
simple_test( vtkDMMLDisplayableNodeTest1 )
simple_test( vtkDMMLDisplayNodeTest1 )
simple_test( vtkDMMLGlyphableVolumeDisplayNodeTest1 )
simple_test( vtkDMMLGlyphableVolumeSliceDisplayNodeTest1 )
simple_test( vtkDMMLGridTransformNodeTest1 )
simple_test( vtkDMMLHierarchyNodeTest1 )
simple_test( vtkDMMLHierarchyNodeTest3 )
simple_test( vtkDMMLDisplayableHierarchyNodeDisplayPropertiesTest )
simple_test( vtkDMMLDisplayableHierarchyNodeTest1 )
simple_test( vtkDMMLDisplayableHierarchyNodeTest2 )
simple_test( vtkDMMLDisplayableHierarchyNodeTest3 )
simple_test( vtkDMMLInteractionNodeTest1 )
simple_test( vtkDMMLLabelMapVolumeDisplayNodeTest1 )
simple_test( vtkDMMLLayoutNodeTest1 )
simple_test( vtkDMMLLinearTransformNodeTest1 )
simple_test( vtkDMMLModelDisplayNodeTest1 )
simple_test( vtkDMMLModelHierarchyNodeTest1 )
simple_test( vtkDMMLModelNodeTest1 )
simple_test( vtkDMMLModelStorageNodeTest1 ${TEMP})
simple_test( vtkDMMLNodeTest1 )
simple_test( vtkDMMLLinearTransformNodeEventsTest )
simple_test( vtkDMMLNonlinearTransformNodeTest1 ${CMAKE_CURRENT_SOURCE_DIR}/NonLinearTransformScene.dmml)
simple_test( vtkDMMLNRRDStorageNodeTest1 )
simple_test( vtkDMMLPETProceduralColorNodeTest1 )
simple_test( vtkDMMLPlotChartNodeTest1 )
simple_test( vtkDMMLPlotSeriesNodeTest1 )
simple_test( vtkDMMLPlotViewNodeTest1 )
simple_test( vtkDMMLProceduralColorNodeTest1 )
simple_test( vtkDMMLProceduralColorStorageNodeTest1 )
simple_test( vtkDMMLROIListNodeTest1 )
simple_test( vtkDMMLROINodeTest1 )
simple_test( vtkDMMLScalarVolumeDisplayNodeTest1 )
simple_test( vtkDMMLScalarVolumeNodeTest1 )
simple_test( vtkDMMLScalarVolumeNodeTest2 )
simple_test( vtkDMMLSceneAddSingletonTest )
simple_test( vtkDMMLSceneBatchProcessTest )
simple_test( vtkDMMLSceneImportIDConflictTest )
simple_test( vtkDMMLSceneImportIDModelHierarchyConflictTest )
simple_test( vtkDMMLSceneImportIDModelHierarchyParentIDConflictTest )
simple_test( vtkDMMLSceneIDTest )
simple_test( vtkDMMLSceneTest1 )
simple_test( vtkDMMLSceneDefaultNodeTest )
# Disabled scene view tests for now - they will be fixed in upcoming commit
# simple_test( vtkDMMLSceneViewNodeImportSceneTest )
# simple_test( vtkDMMLSceneViewNodeEventsTest )
# simple_test( vtkDMMLSceneViewNodeRestoreSceneTest )
# simple_test( vtkDMMLSceneViewNodeStoreSceneTest )
simple_test( vtkDMMLSceneViewNodeTest1 )
simple_test( vtkDMMLSceneViewStorageNodeTest1 )
simple_test( vtkDMMLSegmentationStorageNodeTest1
  DATA{${INPUT}/ITKSnapSegmentation.nii.gz}
  DATA{${INPUT}/OldCjyxSegmentation.seg.nrrd}
  DATA{${INPUT}/CjyxSegmentation.seg.nrrd}
  )
simple_test( vtkDMMLSelectionNodeTest1 )
simple_test( vtkDMMLSliceCompositeNodeTest1 )
simple_test( vtkDMMLSliceNodeTest1 )
simple_test( vtkDMMLSnapshotClipNodeTest1 )
simple_test( vtkDMMLStorableNodeTest1 )
simple_test( vtkDMMLStorageNodeTest1 )
simple_test( vtkDMMLStreamingVolumeNodeTest1 )
simple_test( vtkDMMLTableNodeTest1 )
simple_test( vtkDMMLTableStorageNodeTest1 ${TEMP})
simple_test( vtkDMMLTableViewNodeTest1 )
simple_test( vtkDMMLTensorVolumeNodeTest1 )
simple_test( vtkDMMLTextNodeTest1 )
simple_test( vtkDMMLTextStorageNodeTest1 ${TEMP})
simple_test( vtkDMMLTransformableNodeReferenceSaveImportTest )
simple_test( vtkDMMLTransformableNodeOnNodeReferenceAddTest )
simple_test( vtkDMMLTransformableNodeTest1 )
simple_test( vtkDMMLTransformDisplayNodeTest1 )
simple_test( vtkDMMLTransformNodeTest1 )
simple_test( vtkDMMLTransformStorageNodeTest1 )
simple_test( vtkDMMLUnitNodeTest1 )
simple_test( vtkDMMLVectorVolumeDisplayNodeTest1 )
simple_test( vtkDMMLVectorVolumeNodeTest1 )
simple_test( vtkDMMLViewNodeTest1 )
simple_test( vtkDMMLVolumeArchetypeStorageNodeTest1  ${TEMP})
simple_test( vtkDMMLVolumeDisplayNodeTest1 )
simple_test( vtkDMMLVolumeHeaderlessStorageNodeTest1 )
simple_test( vtkDMMLVolumeNodeEventsTest )
simple_test( vtkDMMLVolumeNodeTest1 )
simple_test( vtkArchiveTest1 DATA{${INPUT}/vol.zip} )
simple_test( vtkCodedEntryTest1 )
simple_test( vtkObserverManagerTest1 )
simple_test( vtkOrientedBSplineTransformTest1 )
simple_test( vtkOrientedGridTransformTest1 )
simple_test( vtkThinPlateSplineTransformTest1 )

function(SIMPLE_TEST_WITH_SCENE TESTNAME SCENEFILENAME)
  # Extract list of external files to download. Note that the ${_externalfiles} variable
  # is only specified to trigger download of data files used in the scene, the arguments
  # are NOT explcitly used in ${KIT}CxxTests test executable.
  string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
  list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
  list(LENGTH scenefilename_and_externalfiles _length)
  if(_length EQUAL 2)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${SCENEFILENAME}
      ${CMAKE_CURRENT_BINARY_DIR}/${SCENEFILENAME}
      COPYONLY
      )
    set(_scenefile ${CMAKE_CURRENT_BINARY_DIR}/${SCENEFILENAME})
    list(GET scenefilename_and_externalfiles 1 _externalfiles)
  else()
    set(_scenefile ${CMAKE_CURRENT_SOURCE_DIR}/${SCENEFILENAME})
    set(_externalfiles "")
  endif()
  simple_test(${TESTNAME}_${SCENEFILENAME} DRIVER_TESTNAME ${TESTNAME} ${_scenefile} ${_externalfiles})
endfunction()

set( ScenesToTest
  clipCube.dmml
  cube.dmml
  diffusionEditor.dmml|DATA{${INPUT}/helix-DTI.nhdr,helix-DTI.raw.gz,helix-DWI.nhdr,helix-DWI.raw.gz}
  helixGlyphs.dmml|DATA{${INPUT}/helix-DTI.nhdr,helix-DTI.raw.gz}
  hier_test.dmml
  multi_display.dmml|DATA{${INPUT}/cube.vtk}
  name.dmml|DATA{${INPUT}/cube.vtk,fixed.nrrd,moving.nrrd}
  SlicePipeline.dmml|DATA{${INPUT}/fixed.nrrd}
  volumetric_mesh.dmml
  vol_and_cube_camera.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk}
  vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk}
  vol.dmml|DATA{${INPUT}/fixed.nrrd}
  volScene.dmml|DATA{${INPUT}/fixed.nrrd,moving.nrrd}
  )

foreach(SceneToTest ${ScenesToTest})
  SIMPLE_TEST_WITH_SCENE( vtkDMMLSceneTest2 ${SceneToTest} )
  SIMPLE_TEST_WITH_SCENE( vtkDMMLSceneImportTest ${SceneToTest} )
endforeach()
