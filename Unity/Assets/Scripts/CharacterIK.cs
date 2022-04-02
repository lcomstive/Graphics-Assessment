using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Animator))]
public class CharacterIK : MonoBehaviour
{
	[SerializeField] private LayerMask m_IKHitLayers;

	[Header("Foot IK")]
	[SerializeField] private float m_FootRayCheckDistance = 0.25f;
	[SerializeField] private float m_FootBoneDistanceFromGround = 0.25f;

	[Header("Hand IK")]
	[SerializeField] private float m_HandRayCheckDistance = 0.5f;

	private Animator m_Animator;

	private void Start() => m_Animator = GetComponent<Animator>();

	void CheckFootIK(AvatarIKGoal goal, float weight)
	{
		m_Animator.SetIKPositionWeight(goal, weight);
		m_Animator.SetIKRotationWeight(goal, weight);

		Ray ray = new Ray(m_Animator.GetIKPosition(goal) + Vector3.up, Vector3.down);
		if (Physics.Raycast(ray, out RaycastHit hit, m_FootRayCheckDistance + m_FootBoneDistanceFromGround + 1.0f, m_IKHitLayers.value))
		{
			Vector3 footPos = hit.point;
			footPos.y += m_FootBoneDistanceFromGround;
			m_Animator.SetIKPosition(goal, footPos);
			m_Animator.SetIKRotation(goal, Quaternion.LookRotation(transform.forward, hit.normal));
		}
	}

	private void OnAnimatorIK(int _)
	{
		if (!m_Animator)
			return;

		CheckFootIK(AvatarIKGoal.LeftFoot, m_Animator.GetFloat("IKLeftFootWeight"));
		CheckFootIK(AvatarIKGoal.RightFoot, m_Animator.GetFloat("IKRightFootWeight"));
	}
}
