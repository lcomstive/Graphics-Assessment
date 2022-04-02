using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Collider))]
public class WavePlayerRespawner : MonoBehaviour
{
	private void Start() => GetComponent<Collider>().isTrigger = true;

	private void OnTriggerEnter(Collider other)
	{
		if(other.TryGetComponent(out PlayerRespawn respawner))
			respawner.Respawn();
	}
}
