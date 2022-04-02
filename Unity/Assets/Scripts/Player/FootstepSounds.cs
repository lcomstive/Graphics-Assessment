using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FootstepSounds : MonoBehaviour
{
	[SerializeField] private AudioSource m_LeftFootAudioSource;
	[SerializeField] private AudioSource m_RightFootAudioSource;
	private PlayerController m_PlayerController;

	private void Start() => m_PlayerController = GetComponent<PlayerController>();

	/// <param name="foot">-1 = left. 1 = right</param>
	private void OnAnimationFootLand(int foot)
	{
		GroundInfo info = m_PlayerController.GroundInfo;
		if(!info)
			return;

		AudioSource source = foot >= 0 ? m_RightFootAudioSource : m_LeftFootAudioSource;

		source.PlayOneShot(
			info.Sounds[Random.Range(0, info.Sounds.Length)],
			Random.Range(info.VolumeVariation.x, info.VolumeVariation.y)
		);
	}
}
