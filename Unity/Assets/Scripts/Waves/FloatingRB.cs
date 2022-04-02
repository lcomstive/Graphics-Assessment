using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
public class FloatingRB : MonoBehaviour
{
	[Header("Air")]
	[SerializeField] private float m_AirDrag = 0.0f;
	[SerializeField] private float m_AirAngularDrag = 0.05f;
	[Header("Under Water")]
	[SerializeField] private float m_WaterDrag = 3.0f;
	[SerializeField] private float m_WaterAngularDrag = 1.0f;
	[Space()]
	
	[SerializeField, Tooltip("Dampens the buoyancy force applied")]
	private float m_DampeningFactor = 0.1f;
	
	[SerializeField] private float m_Buoyancy = 15.0f;
	[SerializeField] private Transform[] m_BouyancyPoints;

	private bool m_Underwater = false;
    private WaveMesh m_WaveMesh;
    private Rigidbody m_Rigidbody;

	private void Awake() => m_Rigidbody = GetComponent<Rigidbody>();

	private void FixedUpdate()
	{
		if(!m_WaveMesh)
		{
			if(m_Underwater)
				SetUnderwater(false);
			return;
		}

		bool hasUnderwaterPoint = false;
		foreach(Transform point in m_BouyancyPoints)
		{
			Vector3 force = GetBuoyancyForce(point.position);

			if (force.y <= 0)
				continue; // No buoyancy force applied
			hasUnderwaterPoint = true;

			m_Rigidbody.AddForceAtPosition(force, point.position, ForceMode.Force);
			m_Rigidbody.AddForceAtPosition(-m_Rigidbody.velocity * m_DampeningFactor, point.position, ForceMode.Force);

			if(!m_Underwater)
				SetUnderwater(true);
		}

		if(m_Underwater && !hasUnderwaterPoint)
			SetUnderwater(false);
	}

	private void SetUnderwater(bool underwater)
	{
		m_Underwater = underwater;
		m_Rigidbody.drag = underwater ? m_WaterDrag : m_AirDrag;
		m_Rigidbody.angularDrag = underwater ? m_WaterAngularDrag : m_AirAngularDrag;
	}

	private void OnTriggerEnter(Collider other)
	{
		if(other.TryGetComponent(out WaveMesh waveMesh))
			m_WaveMesh = waveMesh;
	}

	private void OnTriggerExit(Collider other)
	{
		if(other.gameObject == m_WaveMesh.gameObject)
			m_WaveMesh = null;
		SetUnderwater(false);
	}

	private Vector3 GetBuoyancyForce(Vector3 position)
	{
		if(!m_WaveMesh) return Vector3.zero;

		float waveDelta = position.y - m_WaveMesh.GetWaveHeight(position);
		if(waveDelta >= 0) return Vector3.zero; // Wave is at or above position

		return Vector3.up * m_Buoyancy *
				-waveDelta * Physics.gravity.magnitude *
				1.0f / m_WaveMesh.Viscosity;
	}
}
