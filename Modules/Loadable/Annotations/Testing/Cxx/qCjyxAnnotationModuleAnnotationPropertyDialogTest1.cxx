#include "qCjyxAnnotationModuleAnnotationPropertyDialog.h"
#include "vtkCjyxAnnotationModuleLogic.h"
#include "vtkDMMLAnnotationNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationLineDisplayNode.h"
#include "vtkDMMLAnnotationTextDisplayNode.h"
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLAnnotationRulerNode.h"

// VTK includes
#include <vtkSmartPointer.h>

vtkSmartPointer<vtkDMMLAnnotationRulerNode> m_rulerCopy;
vtkSmartPointer<vtkDMMLAnnotationDisplayNode> m_textDispCopy;
vtkSmartPointer<vtkDMMLAnnotationLineDisplayNode> m_lineDispCopy;
vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode> m_pointDispCopy;

void SaveLinesNode(vtkDMMLAnnotationLinesNode* node);
void SaveControlPoints(vtkDMMLAnnotationControlPointsNode* node);
void SaveAnnotationNode(vtkDMMLAnnotationNode* node);
void UndoLinesNode(vtkDMMLAnnotationLinesNode* node);
void UndoControlPoints(vtkDMMLAnnotationControlPointsNode* node);
void UndoAnnotationNode(vtkDMMLAnnotationNode* node);

//-----------------------------------------------------------------------------
void SaveLinesNode(vtkDMMLAnnotationLinesNode* node)
{
    if (!node)
    {
        return;
    }
    if (!m_lineDispCopy)
    {
        m_lineDispCopy = vtkSmartPointer<vtkDMMLAnnotationLineDisplayNode>::New();
    }

    node->CreateAnnotationLineDisplayNode();
    m_lineDispCopy->Copy(node->GetAnnotationLineDisplayNode());
    SaveControlPoints(node);

}

//-----------------------------------------------------------------------------
void SaveControlPoints(vtkDMMLAnnotationControlPointsNode* node)
{
    if (!node)
    {
        return;
    }

    if (!m_pointDispCopy)
    {
        m_pointDispCopy = vtkSmartPointer<vtkDMMLAnnotationPointDisplayNode>::New();
    }
    node->CreateAnnotationPointDisplayNode();
    m_pointDispCopy->Copy(node->GetAnnotationPointDisplayNode());
    SaveAnnotationNode( (vtkDMMLAnnotationNode*) node);
}

//-----------------------------------------------------------------------------
void SaveAnnotationNode(vtkDMMLAnnotationNode* node)
{
    if (!node)
    {
        return;
    }

    if (!m_textDispCopy)
    {
        m_textDispCopy = vtkSmartPointer<vtkDMMLAnnotationTextDisplayNode>::New();
    }
    node->CreateAnnotationTextDisplayNode();
    m_textDispCopy->Copy(node->GetAnnotationTextDisplayNode());
}

//-----------------------------------------------------------------------------
void SaveStateForUndo(vtkDMMLNode* node)
{
    if (node->IsA( "vtkDMMLAnnotationRulerNode" ))
    {
        vtkDMMLAnnotationRulerNode* mynode = vtkDMMLAnnotationRulerNode::SafeDownCast(node);
        if (!m_rulerCopy)
        {
            m_rulerCopy = vtkSmartPointer<vtkDMMLAnnotationRulerNode>::New();
        }
        m_rulerCopy->Copy(mynode);
        SaveLinesNode(mynode);
    }

}

//-----------------------------------------------------------------------------
void UndoLinesNode(vtkDMMLAnnotationLinesNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationLineDisplayNode();
    node->GetAnnotationLineDisplayNode()->Copy(m_lineDispCopy);
    UndoControlPoints(node);
}

//-----------------------------------------------------------------------------
void UndoControlPoints(vtkDMMLAnnotationControlPointsNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationPointDisplayNode();
    node->GetAnnotationPointDisplayNode()->Copy(m_pointDispCopy);
    UndoAnnotationNode( (vtkDMMLAnnotationNode*) node);
}

//-----------------------------------------------------------------------------
void UndoAnnotationNode(vtkDMMLAnnotationNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationTextDisplayNode();
    node->GetAnnotationTextDisplayNode()->Copy(m_textDispCopy);
}

//-----------------------------------------------------------------------------
void Undo(vtkDMMLNode* node)
{
    if (node->IsA( "vtkDMMLAnnotationRulerNode" ))
    {
        vtkDMMLAnnotationRulerNode* rnode = vtkDMMLAnnotationRulerNode::SafeDownCast(node);
        rnode->Copy(m_rulerCopy);
        UndoLinesNode(rnode);
    }
    else if (node->IsA( "vtkDMMLAnnotationFiducialNode" ))
    {
        //ToDo
    }

}

//-----------------------------------------------------------------------------
int qCjyxAnnotationModuleAnnotationPropertyDialogTest1( int, char * [] )
{
    // Basic Setup
    vtkSmartPointer<vtkCjyxAnnotationModuleLogic > pLogic = vtkSmartPointer< vtkCjyxAnnotationModuleLogic >::New();
    vtkSmartPointer<vtkDMMLScene> pDMMLScene = vtkSmartPointer<vtkDMMLScene>::New();
    vtkSmartPointer<vtkDMMLAnnotationRulerNode> pRulerNode = vtkSmartPointer<vtkDMMLAnnotationRulerNode>::New();
    pDMMLScene->RegisterNodeClass(pRulerNode);
    pDMMLScene->AddNode(pRulerNode);

    QString textString = "MyTestString";
    pLogic->SetAnnotationLinesProperties( (vtkDMMLAnnotationLinesNode*)pRulerNode, vtkCjyxAnnotationModuleLogic::TEXT, textString.toUtf8());

    QString text1 = QString(pLogic->GetAnnotationTextProperty(pRulerNode));

    SaveStateForUndo(pRulerNode);

    QString textString2 = "AnotherTestString";
    pLogic->SetAnnotationLinesProperties( (vtkDMMLAnnotationLinesNode*)pRulerNode, vtkCjyxAnnotationModuleLogic::TEXT, textString2.toUtf8());
    QString text22 = QString(pLogic->GetAnnotationTextProperty(pRulerNode));
    std::cout << qPrintable(text22) << std::endl;

    Undo((vtkDMMLNode*)pRulerNode);

    QString text2 = QString(pLogic->GetAnnotationTextProperty(pRulerNode));

    if ( text1 == text2 )
    {
        std::cout << "SaveStateForUndo/Undo okay!" << std::endl;
        std::cout << qPrintable(text1) << std::endl;
        return 0;
    }

    std::cout << "SaveStateForUndo/Undo Failed!" << std::endl;
    std::cout << "Text 1:" << qPrintable(text2) << " Text 2: " <<qPrintable(text2) << std::endl;
    return 1;
}


