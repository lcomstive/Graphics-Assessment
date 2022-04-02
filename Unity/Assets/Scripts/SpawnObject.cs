using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpawnObject : MonoBehaviour
{
	[SerializeField] private GameObject[] m_Prefabs;
	[SerializeField] private Vector3 m_SpawnOffset;

	[Header("Audio")]
	[SerializeField] private AudioClip m_Clip;
	[SerializeField] private AudioSource m_AudioSource;
	[SerializeField] private Vector2 m_AudioPitchRange;

    public void Spawn()
	{
		int index = Random.Range(0, m_Prefabs.Length);
		GameObject spawned = Instantiate(m_Prefabs[index]);

		spawned.transform.position = transform.position + transform.TransformDirection(m_SpawnOffset);
		spawned.transform.rotation = m_Prefabs[index].transform.rotation;
		spawned.transform.localScale = m_Prefabs[index].transform.localScale;

		if(m_AudioSource && m_Clip)
		{
			m_AudioSource.pitch = Random.Range(m_AudioPitchRange.x, m_AudioPitchRange.y);
			m_AudioSource.PlayOneShot(m_Clip);
		}
	}

	private void OnDrawGizmos()
	{
		Gizmos.color = Color.green;
		Gizmos.DrawWireSphere(transform.position + m_SpawnOffset, 0.1f);
	}
}
