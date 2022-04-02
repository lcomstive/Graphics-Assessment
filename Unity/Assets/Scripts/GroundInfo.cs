using UnityEngine;

[CreateAssetMenu(fileName = "Ground", menuName = "Ground Info")]
public class GroundInfo : ScriptableObject
{
	public AudioClip[] Sounds;
	public Vector2 VolumeVariation = new Vector2(0.8f, 1.2f);
}
