using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GroundInfoHolder : MonoBehaviour
{
	[SerializeField] private GroundInfo m_Info;

	public GroundInfo Info => m_Info;
}
