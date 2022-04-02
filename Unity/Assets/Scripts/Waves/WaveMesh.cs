using UnityEngine;

[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
public class WaveMesh : MonoBehaviour
{
	[SerializeField] private float m_Viscosity = 1.0f;
	[SerializeField] private float m_Amplitude = 1.0f;
	[SerializeField] private float m_Length = 1.0f;
	[SerializeField] private float m_Speed = 1.0f;

	public float Viscosity => m_Viscosity;

	private Material m_Material;

	private void Awake()
	{
#if UNITY_EDITOR // Only execute in editor
		bool hasTrigger = false;
		Collider[] colliders = GetComponentsInChildren<Collider>();
		foreach (Collider collider in colliders)
		{
			if (collider.isTrigger)
			{
				hasTrigger = true;
				break;
			}
		}

		if (!hasTrigger)
			Debug.LogWarning("Don't forget to add a trigger to your wave mesh!");
#endif

		m_Material = GetComponent<MeshRenderer>().material;
		UpdateShader();
	}

#if UNITY_EDITOR
	[ExecuteInEditMode]
	private void Update() => UpdateShader();
#endif

	public void UpdateShader()
	{
		m_Material.SetFloat("_Wave_Speed",		m_Speed);
		m_Material.SetFloat("_Wave_Length",		m_Length);
		m_Material.SetFloat("_Wave_Amplitude",	m_Amplitude);
	}

	public float GetWaveHeight(Vector3 position) => m_Amplitude * Mathf.Sin(position.x / m_Length + Time.time);
}
