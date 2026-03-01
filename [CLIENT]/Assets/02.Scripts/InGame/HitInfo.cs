using UnityEngine;

namespace _02.Scripts.InGame
{
    public readonly struct HitInfo
    {
        public readonly int Damage;
        public readonly Vector2 Direction;

        public HitInfo(int damage, Vector2 direction)
        {
            Damage = damage;
            Direction = direction;
        }
    }
}
